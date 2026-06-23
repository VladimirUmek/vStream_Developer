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

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include "cmsis_vstream.h"

/* References to the underlying CMSIS vStream driver */
extern vStreamDriver_t            Driver_vStreamAudioIn;
extern vStreamDriver_t            Driver_vStreamAudioOut;

#define vStream_AudioIn         (&Driver_vStreamAudioIn)
#define vStream_AudioOut        (&Driver_vStreamAudioOut)

#endif /* APP_MAIN_H_ */
