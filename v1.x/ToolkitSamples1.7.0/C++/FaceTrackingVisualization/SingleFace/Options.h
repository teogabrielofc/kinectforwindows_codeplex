//------------------------------------------------------------------------------
// <copyright file="Options.h" company="Microsoft">
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


#pragma once

#ifndef _SAMPLE_DEBUG_OPTIONS_
#define _SAMPLE_DEBUG_OPTIONS_

#include "CreateOptions.h"
FT_CREATE_OPTIONS_V1 _g_val(FT_CREATE_OPTIONS_FLAGS_DEBUG_DEPTH_MASK);
PVOID _opt = &_g_val;  

#endif //_SAMPLE_DEBUG_OPTIONS_
