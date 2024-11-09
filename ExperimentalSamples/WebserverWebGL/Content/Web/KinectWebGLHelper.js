// -----------------------------------------------------------------------
// <copyright file="KinectWebGLHelper.js" company="Microsoft">
// 
//	 Copyright 2013 Microsoft Corporation 
// 	 
//	Licensed under the Apache License, Version 2.0 (the "License"); 
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
// 	 
//		 http://www.apache.org/licenses/LICENSE-2.0 
// 	 
//	Unless required by applicable law or agreed to in writing, software 
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//	See the License for the specific language governing permissions and 
//	limitations under the License. 
// 
// </copyright>
// -----------------------------------------------------------------------

function KinectWebGLHelper(sensor) {
    "use strict";

    //////////////////////////////////////////////////////////////
    // ImageMetadata object constructor
    function ImageMetadata(imageCanvas) {
        
        //////////////////////////////////////////////////////////////
        // ImageMetadata private constants

        // vertices representing entire viewport as two triangles which make up the whole
        // rectangle, in post-projection/clipspace coordinates
        var VIEWPORT_VERTICES = new Float32Array([
            -1.0, -1.0,
            1.0, -1.0,
            -1.0, 1.0,
            -1.0, 1.0,
            1.0, -1.0,
            1.0, 1.0]);
        var NUM_VIEWPORT_VERTICES = VIEWPORT_VERTICES.length / 2;
        
        // Texture coordinates corresponding to each viewport vertex
        var VERTEX_TEXTURE_COORDS = new Float32Array([
            0.0, 1.0,
            1.0, 1.0,
            0.0, 0.0,
            0.0, 0.0,
            1.0, 1.0,
            1.0, 0.0]);
        
        // Convolution kernel weights (blurring effect by default)
        var CONVOLUTION_KERNEL_WEIGHTS = new Float32Array([
            1, 1, 1,
            1, 1, 1,
            1, 1, 1]);
        var TOTAL_WEIGHT = 0;
        for (var i = 0; i < CONVOLUTION_KERNEL_WEIGHTS.length; ++i) {
            TOTAL_WEIGHT += CONVOLUTION_KERNEL_WEIGHTS[i];
        }

        //////////////////////////////////////////////////////////////
        // ImageMetadata private properties
        var metadata = this;
        var contextAttributes = { premultipliedAlpha: true };
        var glContext = imageCanvas.getContext('webgl', contextAttributes) || imageCanvas.getContext('experimental-webgl', contextAttributes);
        glContext.clearColor(0.0, 0.0, 0.0, 0.0);      // Set clear color to black, fully transparent
        
        var vertexShader = createShaderFromSource(glContext.VERTEX_SHADER,
            "\
            attribute vec2 aPosition;\
            attribute vec2 aTextureCoord;\
            \
            varying highp vec2 vTextureCoord;\
            \
            void main() {\
                gl_Position = vec4(aPosition, 0, 1);\
                vTextureCoord = aTextureCoord;\
            }");
        var fragmentShader = createShaderFromSource(glContext.FRAGMENT_SHADER,
            "\
            precision mediump float;\
            \
            varying highp vec2 vTextureCoord;\
            \
            uniform sampler2D uSampler;\
            uniform float uWeights[9];\
            uniform float uTotalWeight;\
            \
            /* Each sampled texture coordinate is 2 pixels appart rather than 1, to make filter effects more noticeable. */ \
            const float xInc = 2.0/640.0;\
            const float yInc = 2.0/480.0;\
            const int numElements = 9;\
            const int numCols = 3;\
            \
            void main() {\
                vec4 centerColor = texture2D(uSampler, vTextureCoord);\
                vec4 totalColor = vec4(0,0,0,0);\
                \
                for (int i = 0; i < numElements; i++) {\
                    int iRow = i / numCols;\
                    int iCol = i - (numCols * iRow);\
                    float xOff = float(iCol - 1) * xInc;\
                    float yOff = float(iRow - 1) * yInc;\
                    vec4 colorComponent = texture2D(uSampler, vec2(vTextureCoord.x+xOff, vTextureCoord.y+yOff));\
                    totalColor += (uWeights[i] * colorComponent);\
                }\
                \
                float effectiveWeight = uTotalWeight;\
                if (uTotalWeight <= 0.0) {\
                    effectiveWeight = 1.0;\
                }\
                /* Premultiply colors with alpha component for center pixel. */\
                gl_FragColor = vec4(totalColor.rgb * centerColor.a / effectiveWeight, centerColor.a);\
            }");
        var program = createProgram([vertexShader, fragmentShader]);
        glContext.useProgram(program);

        var positionAttribute = glContext.getAttribLocation(program, "aPosition");
        glContext.enableVertexAttribArray(positionAttribute);

        var textureCoordAttribute = glContext.getAttribLocation(program, "aTextureCoord");
        glContext.enableVertexAttribArray(textureCoordAttribute);
        
        // Associate the uniform texture sampler with TEXTURE0 slot
        var textureSamplerUniform = glContext.getUniformLocation(program, "uSampler");
        glContext.uniform1i(textureSamplerUniform, 0);
        
        // Associate the uniform convolution kernel weights with
        var convolutionKernelWeightsUniform = glContext.getUniformLocation(program, "uWeights[0]");
        glContext.uniform1fv(convolutionKernelWeightsUniform, CONVOLUTION_KERNEL_WEIGHTS);
        
        var convolutionKernelTotalWeightUniform = glContext.getUniformLocation(program, "uTotalWeight");
        glContext.uniform1f(convolutionKernelTotalWeightUniform, TOTAL_WEIGHT);

        // Create a buffer used to represent whole set of viewport vertices
        var vertexBuffer = glContext.createBuffer();
        glContext.bindBuffer(glContext.ARRAY_BUFFER, vertexBuffer);
        glContext.bufferData(glContext.ARRAY_BUFFER, VIEWPORT_VERTICES, glContext.STATIC_DRAW);
        glContext.vertexAttribPointer(positionAttribute, 2, glContext.FLOAT, false, 0, 0);
        
        // Create a buffer used to represent whole set of vertex texture coordinates
        var textureCoordinateBuffer = glContext.createBuffer();
        glContext.bindBuffer(glContext.ARRAY_BUFFER, textureCoordinateBuffer);
        glContext.bufferData(glContext.ARRAY_BUFFER, VERTEX_TEXTURE_COORDS, glContext.STATIC_DRAW);
        glContext.vertexAttribPointer(textureCoordAttribute, 2, glContext.FLOAT, false, 0, 0);

        // Create a texture to contain images from Kinect server
        // Note: TEXTURE_MIN_FILTER, TEXTURE_WRAP_S and TEXTURE_WRAP_T parameters need to be set
        //       so we can handle textures whose width and height are not a power of 2.
        var texture = glContext.createTexture();
        glContext.bindTexture(glContext.TEXTURE_2D, texture);
        glContext.texParameteri(glContext.TEXTURE_2D, glContext.TEXTURE_MAG_FILTER, glContext.LINEAR);
        glContext.texParameteri(glContext.TEXTURE_2D, glContext.TEXTURE_MIN_FILTER, glContext.LINEAR);
        glContext.texParameteri(glContext.TEXTURE_2D, glContext.TEXTURE_WRAP_S, glContext.CLAMP_TO_EDGE);
        glContext.texParameteri(glContext.TEXTURE_2D, glContext.TEXTURE_WRAP_T, glContext.CLAMP_TO_EDGE);
        glContext.bindTexture(glContext.TEXTURE_2D, null);
        
        // Since we're only using one single texture, we just make TEXTURE0 the active one
        // at all times
        glContext.activeTexture(glContext.TEXTURE0);

        //////////////////////////////////////////////////////////////
        // ImageMetadata private methods

        // Create a shader of specified type, with the specified source, and compile it.
        //     .createShaderFromSource(shaderType, shaderSource)
        //
        // shaderType: Type of shader to create (fragment or vertex shader)
        // shaderSource: Source for shader to create (string)
        function createShaderFromSource(shaderType, shaderSource) {
            var shader = glContext.createShader(shaderType);
            glContext.shaderSource(shader, shaderSource);
            glContext.compileShader(shader);

            // Check for errors during compilation
            var status = glContext.getShaderParameter(shader, glContext.COMPILE_STATUS);
            if (!status) {
                var infoLog = glContext.getShaderInfoLog(shader);
                console.log("Unable to compile Kinect '" + shaderType + "' shader. Error:" + infoLog);
                glContext.deleteShader(shader);
                return null;
            }

            return shader;
        }
        
        // Create a WebGL program attached to the specified shaders.
        //     .createProgram(shaderArray)
        //
        // shaderArray: Array of shaders to attach to program
        function createProgram(shaderArray) {
            var newProgram = glContext.createProgram();
            
            for (var shaderIndex = 0; shaderIndex < shaderArray.length; ++shaderIndex) {
                glContext.attachShader(newProgram, shaderArray[shaderIndex]);
            }
            glContext.linkProgram(newProgram);
            
            // Check for errors during linking
            var status = glContext.getProgramParameter(newProgram, glContext.LINK_STATUS);
            if (!status) {
                var infoLog = glContext.getProgramInfoLog(newProgram);
                console.log("Unable to link Kinect WebGL program. Error:" + infoLog);
                glContext.deleteProgram(newProgram);
                return null;
            }

            return newProgram;
        }

        //////////////////////////////////////////////////////////////
        // ImageMetadata public properties
        this.isProcessing = false;
        this.canvas = imageCanvas;
        this.width = 0;
        this.height = 0;
        this.gl = glContext;
        
        //////////////////////////////////////////////////////////////
        // ImageMetadata public functions
        
        // Draw image data into WebGL canvas context
        //     .processImageData(imageBuffer, width, height)
        //
        // imageBuffer: ArrayBuffer containing image data
        // width: width of image corresponding to imageBuffer data
        // height: height of image corresponding to imageBuffer data
        this.processImageData = function(imageBuffer, width, height) {
            if ((width != metadata.width) || (height != metadata.height)) {
                // Whenever the image width or height changes, update tracked metadata and canvas
                // viewport dimensions.
                this.width = width;
                this.height = height;
                this.canvas.width = width;
                this.canvas.height = height;
                glContext.viewport(0, 0, width, height);
            }
            
            glContext.bindTexture(glContext.TEXTURE_2D, texture);
            glContext.texImage2D(glContext.TEXTURE_2D, 0, glContext.RGBA, width, height, 0, glContext.RGBA, glContext.UNSIGNED_BYTE, new Uint8Array(imageBuffer));

            glContext.drawArrays(glContext.TRIANGLES, 0, NUM_VIEWPORT_VERTICES);
            glContext.bindTexture(glContext.TEXTURE_2D, null);
        };

        // Clear all image data from WebGL canvas
        //     .clear()
        this.clear = function () {
            glContext.clear(glContext.COLOR_BUFFER_BIT | glContext.DEPTH_BUFFER_BIT);
        };
    }

    //////////////////////////////////////////////////////////////
    // KinectWebGLHelper private properties
    var bindableStreamNames = {};
    bindableStreamNames[Kinect.USERVIEWER_STREAM_NAME] = true;
    bindableStreamNames[Kinect.BACKGROUNDREMOVAL_STREAM_NAME] = true;
    var imageMetadataMap = {};

    //////////////////////////////////////////////////////////////
    // KinectWebGLHelper private functions
    
    // Associate specified stream name with canvas.
    //     .setImageData(streamName, canvas)
    //
    // streamName: Name of image stream to associate with canvas
    // canvas: Canvas to bind to user viewer stream
    function setImageData(streamName, canvas) {
        if (canvas != null) {
            var metadata = new ImageMetadata(canvas);
            imageMetadataMap[streamName] = metadata;
        } else if (imageMetadataMap.hasOwnProperty(streamName)) {
            // If specified canvas is null but we're already tracking image data,
            // remove metadata associated with this image.
            delete imageMetadataMap[streamName];
        }
    }

    // Send named image stream data to be processed by WebGL canvas context
    //     .processImageData(streamName, imageBuffer, width, height)
    //
    // streamName: Stream name corresponding to image to process
    // imageBuffer: ArrayBuffer containing image data
    // width: width of image corresponding to imageBuffer data
    // height: height of image corresponding to imageBuffer data
    function processImageData(streamName, imageBuffer, width, height) {
        if (!imageMetadataMap.hasOwnProperty(streamName)) {
            // We're not tracking this stream, so no work to do
            return;
        }
        var metadata = imageMetadataMap[streamName];

        if (metadata.isProcessing || (width <= 0) || (height <= 0)) {
            // Don't start processing new data when we are in the middle of
            // processing data already.
            // Also, Only do work if image data to process is of the expected size
            return;
        }

        metadata.processImageData(imageBuffer, width, height);
    }

    // Function called back when a sensor stream frame is ready to be processed.
    //     .streamFrameHandler(streamFrame)
    //
    // streamFrame: stream frame ready to be processed
    //
    // Remarks
    // Processes interaction frames and ignores all other kinds of stream frames.
    function streamFrameHandler(streamFrame) {
        if (typeof streamFrame != "object") {
            throw new Error("Frame must be an object");
        }

        if (streamFrame == null) {
            // Ignore null frames
            return;
        }

        var streamName = streamFrame.stream;
        if (bindableStreamNames[streamName]) {
            // If this is one of the bindable stream names
            processImageData(streamName, streamFrame.buffer, streamFrame.width, streamFrame.height);
        }
    }

    //////////////////////////////////////////////////////////////
    // KinectWebGLHelper public functions

    // Bind the specified canvas element with the specified image stream
    //     .bindStreamToCanvas( streamName, canvas )
    //
    // streamName: name of stream to bind to canvas element. Must be one of the supported
    //             image stream names (e.g.: KinectUI.USERVIEWER_STREAM_NAME and
    //             KinectUI.BACKGROUNDREMOVAL_STREAM_NAME)
    // canvas: Canvas to bind to user viewer stream
    //
    // Remarks
    // After binding a stream to a canvas, image data for that stream will
    // be rendered into the canvas whenever a new stream frame arrives.
    this.bindStreamToCanvas = function (streamName, canvas) {
        if (!bindableStreamNames[streamName]) {
            throw new Error("first parameter must be specified and must be one of the supported stream names");
        }

        if (!(canvas instanceof HTMLCanvasElement)) {
            throw new Error("second parameter must be specified and must be a canvas element");
        }

        this.unbindStreamFromCanvas(streamName);

        setImageData(streamName, canvas);
    };

    // Unbind the specified image stream from previously bound canvas element, if any.
    //     .unbindStreamFromCanvas(streamName)
    //
    // streamName: name of stream to unbind from its corresponding canvas element
    this.unbindStreamFromCanvas = function (streamName) {
        setImageData(streamName, null);
    };

    // Get metadata associated with specified stream name
    //     .getMetadata(streamName)
    //
    // streamName: name of stream for which metadata will be returned
    this.getMetadata = function (streamName) {
        if (!imageMetadataMap.hasOwnProperty(streamName)) {
            // We're not tracking this image, so no work to do
            return null;
        }
        return imageMetadataMap[streamName];
    };

    //////////////////////////////////////////////////////////////
    // KinectWebGLHelper initialization code
    sensor.addStreamFrameHandler(streamFrameHandler);
}
