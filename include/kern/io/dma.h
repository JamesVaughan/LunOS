#ifndef KERN_IO_DMA_H
#define KERN_IO_DMA_H

#include <kern/io/io.h>

namespace LunOS
{
	namespace IO
	{
		namespace DMACommands
		{
			enum TransmissionTypes
			{
				DemandMode = 0,
				SingleMode = 1,
				BlockMode = 2,
				CascadeMode = 3
			};

			enum ReadDirection
			{
				ReadUp,
				ReadDown
			};

			enum AutoInitialization
			{
				SingleCycle,
				AutoInitialize
			};

			enum DMAType
			{
				EightBit,
				SixteenBit
			};

			enum ReadWrite
			{
				VerifyTransfer,
				WriterTransfer,
				ReadTransfer
			};
		}

		typedef struct DMAChannel
		{
			bool ChannelInUse;
			unsigned int Address;
		} DMAChannel;

		class DMA
		{
		public:
			static void Init();
		private:
			static const int NumberOfChannels = 8;
			static const unsigned short DMA8BitStart = 0;
			static const unsigned short DMA16BitStart = 0xC0;
			static const unsigned short WriteMode8Bit = 0x0B;
			static const unsigned short WriteMode16Bit = 0xD6;
			static const unsigned short SingleMast8Bit = 0x0A;
			static const unsigned short SingleMast16Bit = 0xD4;
			static const unsigned short ClearByte8Bit = 0x0C;
			static const unsigned short ClearByte16Bit = 0xD8;
			static DMAChannel channels[NumberOfChannels];
			DMA();
			~DMA();
		};
		extern DMA Dma;
	}
}



#endif
