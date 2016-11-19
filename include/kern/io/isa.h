/*
 * isa.h
 *
 *  Created on: 2010-10-03
 *      Author: james
 */

#ifndef ISA_H_
#define ISA_H_

#include <kern/system.hpp>

namespace LunOS
{
	namespace IO
	{
		class ISA
		{
		public:
			static void Init();
		};
	}
}


#endif /* ISA_H_ */
