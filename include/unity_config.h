/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#ifndef NULL
#ifndef __cplusplus
#define NULL (void*)0
#else
#define NULL 0
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define UNITY_SUPPORT_64
#include <stdio.h>
#define UNITY_OUTPUT_CHAR(c) putchar(c)
#define UNITY_OUTPUT_FLUSH() fflush(stdout)
#define UNITY_OUTPUT_START()
#define UNITY_OUTPUT_COMPLETE()


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* UNITY_CONFIG_H */
