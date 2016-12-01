/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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

#include "btUnionFind.h"



btUnionFind::~btUnionFind()
{
	Free();

}

btUnionFind::btUnionFind()
{ 

}

void	btUnionFind::allocate(int N)
{
	m_elements.resize(N);
}
void	btUnionFind::Free()
{
	m_elements.clear();
}


void	btUnionFind::reset(int N)
{
	allocate(N);

	for (int i = 0; i < N; i++) 
	{ 
		m_elements[i].m_id = i; m_elements[i].m_sz = 1; 
	} 
}


class btUnionFindElementSortPredicate
{
	public:

#if 1 // for MMDAgent
		bool operator() ( const btElement& lhs, const btElement& rhs ) const
#else
		bool operator() ( const btElement& lhs, const btElement& rhs )
#endif
		{
			return lhs.m_id < rhs.m_id;
		}
};

///this is a special operation, destroying the content of btUnionFind.
///it sorts the elements, based on island id, in order to make it easy to iterate over islands
void	btUnionFind::sortIslands()
{

	//first store the original body index, and islandId
	int numElements = m_elements.size();
	
	for (int i=0;i<numElements;i++)
	{
		m_elements[i].m_id = find(i);
#ifndef STATIC_SIMULATION_ISLAND_OPTIMIZATION
		m_elements[i].m_sz = i;
#endif //STATIC_SIMULATION_ISLAND_OPTIMIZATION
	}
	
	 // Sort the vector using predicate and std::sort
	  //std::sort(m_elements.begin(), m_elements.end(), btUnionFindElementSortPredicate);
	  m_elements.quickSort(btUnionFindElementSortPredicate());

}
