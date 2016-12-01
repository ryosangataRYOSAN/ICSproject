/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

#define PLUGINFILEVIEWER_NAME "FileViewer"

/* headers */

#include "MMDAgent.h"
#ifdef _WIN32
#include <shellapi.h>
#endif /* _WIN32 */

/* variables */
static bool enable;

/* openDirectory: open directory */
static bool openDirectory(const char *dir)
{
   DIRECTORY *d;
   char *path;

   if (dir == NULL)
      return false;

   d = MMDAgent_opendir(dir);
   if (d == NULL) {
      /* dir not exist */
      return false;
   }
   MMDAgent_closedir(d);

   path = MMDAgent_pathdup_from_application_to_system_locale(dir);
   if (path == NULL)
      return false;
#ifdef _WIN32
   if (ShellExecuteA(NULL, NULL, path, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE) 32) {
      /* error */
      free(path);
      return false;
   }
#endif /* _WIN32 */
   free(path);
   return true;
}

/* extAppStart: start app */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   enable = true;
   mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINFILEVIEWER_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if (enable == true) {
      if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if (MMDAgent_strequal(args, PLUGINFILEVIEWER_NAME)) {
            enable = false;
            mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINFILEVIEWER_NAME);
         }
      } else if (MMDAgent_strequal(type, MMDAGENT_EVENT_KEY) && MMDAgent_strequal(args, "O")) {
         openDirectory(mmdagent->getConfigDirName());
      }
   } else {
      if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if (MMDAgent_strequal(args, PLUGINFILEVIEWER_NAME)) {
            enable = true;
            mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINFILEVIEWER_NAME);
         }
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   enable = false;
}
