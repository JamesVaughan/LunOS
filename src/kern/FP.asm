[BITS 32]

global LoadSSEStatus
LoadSSEStatus:
	push ebp
	mov ebp, esp
	push ebx
	push eax ; we need 32-bits to store this
	STMXCSR dword[esp]
	mov ebx, dword[esp]
	pop eax
	mov eax, ebx
	pop ebx
	pop ebp
retn

global StoreSSEStatus
StoreSSEStatus:
	push ebp
	mov ebp, esp
	LDMXCSR dword[ebp + 8]
	pop ebp
retn