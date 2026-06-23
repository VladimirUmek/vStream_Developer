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

// <h> Audio Input
// ==============

//   <o> Channels
//   <i> Number of audio channels for the input stream.
//   <1=> Mono   (1 channel)
//   <2=> Stereo (2 channels)
//   <i> Default: 2 (Stereo)
#ifndef AUDIO_IN_CHANNELS
#define AUDIO_IN_CHANNELS        2
#endif

//   <o> Bits per Sample
//   <i> Number of bits per audio sample for the input stream.
//   <8=>  8-bit
//   <16=> 16-bit
//   <24=> 24-bit
//   <32=> 32-bit
//   <i> Default: 16 (16-bit)
#ifndef AUDIO_IN_SAMPLE_BITS
#define AUDIO_IN_SAMPLE_BITS     16
#endif

//   <o> Sample Rate [Hz]
//   <i> Number of samples per second for the input stream.
//   <8000=>   8 kHz
//   <16000=>  16 kHz
//   <44100=>  44.1 kHz
//   <48000=>  48 kHz
//   <i> Default: 16000 (16 kHz)
#ifndef AUDIO_IN_SAMPLE_RATE
#define AUDIO_IN_SAMPLE_RATE     16000
#endif

// </h>

// <h> Audio Output
// ===============

//   <o> Channels
//   <i> Number of audio channels for the output stream.
//   <1=> Mono   (1 channel)
//   <2=> Stereo (2 channels)
//   <i> Default: 2 (Stereo)
#ifndef AUDIO_OUT_CHANNELS
#define AUDIO_OUT_CHANNELS       2
#endif

//   <o> Bits per Sample
//   <i> Number of bits per audio sample for the output stream.
//   <8=>  8-bit
//   <16=> 16-bit
//   <24=> 24-bit
//   <32=> 32-bit
//   <i> Default: 16 (16-bit)
#ifndef AUDIO_OUT_SAMPLE_BITS
#define AUDIO_OUT_SAMPLE_BITS    16
#endif

//   <o> Sample Rate [Hz]
//   <i> Number of samples per second for the output stream.
//   <8000=>   8 kHz
//   <16000=>  16 kHz
//   <44100=>  44.1 kHz
//   <48000=>  48 kHz
//   <i> Default: 16000 (16 kHz)
#ifndef AUDIO_OUT_SAMPLE_RATE
#define AUDIO_OUT_SAMPLE_RATE    16000
#endif

// </h>

// <h> Streaming
// ============

//   <o> Capture Mode
//   <i> Audio capture and playback strategy for both input and output streams.
//   <0=> Single Block
//   <1=> Continuous
//   <i> Default: 1 (Continuous)
#ifndef VSTREAM_CAPTURE_MODE
#define VSTREAM_CAPTURE_MODE     1
#endif

// </h>

//------------- <<< end of configuration section >>> ---------------------------

#endif /* VSTREAM_CONFIG_H_ */
