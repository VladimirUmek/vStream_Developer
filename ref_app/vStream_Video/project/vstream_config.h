/*
 * Copyright 2026 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VSTREAM_CONFIG_H_
#define VSTREAM_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h> Video Input
// ==============

//   <o> Format
//   <i> Pixel format delivered by the video input stream.
//   <0=> RAW8  (Bayer, 1 byte/pixel — debayered automatically)
//   <1=> RGB565 (2 bytes/pixel)
//   <2=> RGB888 (3 bytes/pixel)
//   <i> Default: 0 (RAW8)
#ifndef VIDEO_IN_FORMAT
#define VIDEO_IN_FORMAT      1
#endif

//   <o> Frame Width <1-65535>
//   <i> Width of the input video frame in pixels.
//   <i> Default: 1280
#ifndef VIDEO_IN_WIDTH
#define VIDEO_IN_WIDTH       1280
#endif

//   <o> Frame Height <1-65535>
//   <i> Height of the input video frame in pixels.
//   <i> Default: 720
#ifndef VIDEO_IN_HEIGHT
#define VIDEO_IN_HEIGHT      720
#endif

//   <o> Bayer Pattern
//   <i> Bayer filter mosaic pattern of the RAW8 camera sensor.
//   <i> Ignored when Format is not RAW8.
//   <0=> RGGB
//   <1=> BGGR
//   <2=> GRBG
//   <3=> GBRG
//   <i> Default: 2 (GRBG)
#ifndef VIDEO_IN_BAYER_PATTERN
#define VIDEO_IN_BAYER_PATTERN     2
#endif

// </h>

// <h> Video Output
// ===============

//   <o> Format
//   <i> Pixel format expected by the video output stream.
//   <1=> RGB565 (2 bytes/pixel)
//   <2=> RGB888 (3 bytes/pixel)
//   <i> Default: 1 (RGB565)
#ifndef VIDEO_OUT_FORMAT
#define VIDEO_OUT_FORMAT     1
#endif

//   <o> Frame Width <1-65535>
//   <i> Width of the output video frame in pixels.
//   <i> Default: 480
#ifndef VIDEO_OUT_WIDTH
#define VIDEO_OUT_WIDTH      480
#endif

//   <o> Frame Height <1-65535>
//   <i> Height of the output video frame in pixels.
//   <i> Default: 800
#ifndef VIDEO_OUT_HEIGHT
#define VIDEO_OUT_HEIGHT     800
#endif

// </h>

// <h> Streaming
// ============

//   <o> Capture Mode
//   <i> Frame capture and display strategy for both input and output streams.
//   <0=> Single Frame
//   <1=> Continuous
//   <i> Default: 0 (Single Frame)
#ifndef VSTREAM_CAPTURE_MODE
#define VSTREAM_CAPTURE_MODE 0
#endif

// </h>

//------------- <<< end of configuration section >>> ---------------------------

#endif /* VSTREAM_CONFIG_H_ */
