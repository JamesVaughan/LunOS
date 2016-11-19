#include <kern/io/dma.h>

namespace LunOS
{
namespace IO
{
	DMAChannel DMA::channels[NumberOfChannels];

	void DMA::Init()
	{
		int i;
		for(i = 0; i < (DMA::NumberOfChannels / 2); i++)
		{
			channels[i].ChannelInUse = false;
			channels[i+4].ChannelInUse = false;
			outportb(DMA8BitStart + (i * 2), 0);
			outportb(DMA16BitStart + (i * 4), 0);

		}
	}
}// end IO
}// end LunOS
