/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*  All rights reserved.                                             */
/*                                                                   */
/*  Copyright (c) 2012       CUBE370                                 */
/*  All rights reserved.                                             */
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

//*********************************************************************
//	�y�\�t�g���z  MMDAgent-1.2�p�v���O�C�����`�v���W�F�N�g
//	�y�o�[�W�����z0.1.0.0
//	�y����ҁz    CUBE370
//	�y�����HP�z  http://cube370.blog87.fc2.com/
//	�y���J���z    2012/06/01
//	�y�z�z���z    http://cube370.blog87.fc2.com/
//	�y��pwiki�z  http://cube370.wiki.fc2.com/
//	�y������z  Microsoft Visual C++ 2008
//*********************************************************************

/***** define *****/
#define EXPORT extern "C" __declspec(dllexport)
#define PLUGINSKELETON_NAME    "Skeleton"

/***** include *****/
#include "MMDAgent.h"
#include "Skeleton_Thread.h"

/***** variables *****/
static bool enable;
static Skeleton_Thread skeleton_thread;

/***** extAppStart: �v���O�C�����[�h & �X���b�h�J�n *****/
EXPORT void extAppStart(MMDAgent *mmdagent)
{
	skeleton_thread.setupAndStart(mmdagent);

	enable = true;
	mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINSKELETON_NAME);
}

/***** extProcCommand: �R�}���h���b�Z�[�W��M������ *****/
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
	if (enable == true)
	{
		if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE))
		{
			if(MMDAgent_strequal(args, PLUGINSKELETON_NAME))
			{
				enable = false;
				mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINSKELETON_NAME);
			}
		}





	}
	else
	{



		if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE))
		{
			if(MMDAgent_strequal(args, PLUGINSKELETON_NAME))
			{
				enable = true;
				mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINSKELETON_NAME); 
				mmdagent->sendMessage("TEST", "COMEHERE");
			}
		}
	}
}


/***** extAppEnd: �v���O�C���I�� & �X���b�h�J�� *****/
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
	skeleton_thread.stopAndRelease();
}
