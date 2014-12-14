// (c) Copyright Paul Campbell paul@taniwha.com 2013
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by Paul Campbell , who shall
// act as a proxy defined in Section 6 of version 3 of the license.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _TASK_H
#define _TASK_H

__sfr __at (0x93) _XPAGE;

typedef unsigned char u8;

typedef struct task {
	void 	(*callout)(struct task __xdata*);	// offset 0
	unsigned int	time;				// offset 2
	struct task     __xdata*next;			// offset 4
	u8		state;				// offset 6
} task;

void queue_task(task __xdata *, unsigned int);
void queue_task_0(task __xdata *);
void cancel_task(task __xdata * );

#define HZ (32000/256)


#define TASK_IDLE 	0
#define TASK_CALLOUT	1
#define TASK_QUEUED	2

extern void wait_us(u8);
extern unsigned char app(u8);

#endif
