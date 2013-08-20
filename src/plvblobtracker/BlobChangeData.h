/*
 *   Copyright (c) 2007 John Weaver
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#if !defined(_BLOBCHANGEDATA_H_)
#define _BLOBCHANGEDATA_H_

#include <list>
#include <utility>
#include <opencv/cv.h>

namespace plvblobtracker
{
	class BlobChangeData {
		//needed for saving clicks etc and sending efficient over files etc. 
	
		//to save changes of the widget
		public: 
			//todo check whether it will be suitable for >2 ids in 1 blob
			//oldsetup:
			int oldid;
			int newid;
			cv::Point cogs;
			char changetype;
			
			//todo shouldnt it be
			//char changetype;
			//int oldid;
			//int newid1;
			//int newid2;
			//cv::Point cogs;
			
	};
}

#endif /* !defined(_BLOBCHANGADATA_H_) */
