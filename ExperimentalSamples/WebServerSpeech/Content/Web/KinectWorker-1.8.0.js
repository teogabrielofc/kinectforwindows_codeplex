//------------------------------------------------------------------------------
// <copyright file="KinectWorker-1.8.0.js" company="Microsoft">
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
//------------------------------------------------------------------------------
"use strict";

var imageDataMap = {};

// Associate a specified ImageData (from a canvas object) with the specified image name
//     .setImageData( imageName [, imageData] )
//
// imageName: Name used to refer to canvas ImageData object specified.
// imageData: Canvas ImageData to associate with image name. If null, any previously
//            existing association is removed.
function setImageData(imageName, imageData) {
    if ((imageData == null) && imageDataMap.hasOwnProperty(imageName)) {
        delete imageDataMap[imageName];
    } else {
        imageDataMap[imageName] = imageData;
    }
}


// Copy the information in the specified image ArrayBuffer to the ImageData associated
// with the specified name and post the ImageData back to UI thread.
//     .processImageData( imageName, imageData )
//
// imageName: Name used to refer to canvas ImageData object to receive data from
//            ArrayBuffer.
// imageBuffer: ArrayBuffer containing image data to copy to canvas ImageData structure.
function processImageData(imageName, imageBuffer) {
    if (!imageDataMap.hasOwnProperty(imageName)) {
        self.postMessage({ "message": "notProcessed", "imageName": imageName});
        return;
    }

    var imageData = imageDataMap[imageName];
    var pixelArray = imageData.data;
    var newPixelData = new Uint8Array(imageBuffer);

    for (var i = 0; i < pixelArray.length; ++i) {
        pixelArray[i] = newPixelData[i];
    }

    self.postMessage({ "message": "imageReady", "imageName": imageName, "imageData": imageData });
}

// thread message handler
addEventListener('message', function (event) {
    switch (event.data.message) {
        case "setImageData":
            setImageData(event.data.imageName, event.data.imageData);
            break;

        case "processImageData":
            processImageData(event.data.imageName, event.data.imageBuffer);
            break;
    }
});