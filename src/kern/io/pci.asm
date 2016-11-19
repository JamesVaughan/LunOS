[BITS 32]

global ReadPCIServiceDir
ReadPCIServiceDir:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	xor eax, eax ; I'm not sure which BIOS service this is... I think it is 0 though
	xor ebx, ebx ; ebx should be zero
	call far dword[ebp+8] ; Call the PCI Service Directory Entry point
	add ebx, edx ; ebx is the base and edx was an offset
	mov [ebp+16], ebx
	pop ebx
	pop eax
	pop ebp
retn
