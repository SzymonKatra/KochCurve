
global koch

%define CONST_1_2		0x3F000000		; 1/2
%define CONST_1_3		0x3EAAAAAB		; 1/3
%define CONST_2_3		0x3F2AAAAB		; 2/3
%define CONST_SQRT3_6	0x3E93CD3A		; sqrt(3) / 6

; rdi - pointer to output buffer
; rsi - pointer to input buffer
; rdx - input points count
%define VAR_TMP8		rbp - 8			; temporary 8 byte buffer
koch:		
										; --- PROLOGUE ---
				push	rbp				; preserve frame pointer
				mov		rbp, rsp		; set up new frame pointer
				sub		rsp, 8			; allocate local variables
				
										; Symbols:
										; A - start point
										; B - end point
										; U - vector between A and B
										; V - perpendicular vector to U
										;
										;	     Q
										;        /\
										;       /  \
										; A___P/    \R___B
										;
										; U = [Bx - Ax, By - Ay]
										; V = [By - Ay, Ax - Bx]
										; P = A + (1/3) * U
										; Q = A + (1/2) * U + (sqrt(3) / 6) * V
										; R = A + (2/3) * U
										
										
				dec		rdx				; we don't process last point
				
koch_loop:
										; --- COMPUTE U AND V ---
				movlps	xmm0, [rsi + 8] ; move Bx, By to low part of xmm0
				mov		eax, [rsi + 12] ; store temporarily By in eax
				mov		[VAR_TMP8], eax ; move By to temp buffer
				mov		eax, [rsi]		; store temporarily Ax in eax
				mov		[VAR_TMP8 + 4], eax ; move Ax into temp buffer
				movhps	xmm0, [VAR_TMP8] ; Move By, Ax to high part of xmm0
				
				movlps	xmm1, [rsi]		; move Ax, Ay to low part of xmm1
				movhps	xmm1, [rsi + 4]	; move Ay, Bx to high part of xmm1
				
				subps	xmm0, xmm1		; compute U and V values
										; xmm0 - (low) [Ux, Uy, Vx, Vy] (high)
										
										; --- COMPUTE P AND R ---
				movaps	xmm1, xmm0		; move Ux, Uy to xmm1 (moved also high part but we don't care)
				movlhps	xmm1, xmm0		; move Ux, Uy to high part of xmm1
				
				mov		dword [VAR_TMP8], CONST_1_3 ; move 1/3 to temp buffer
				mov		dword [VAR_TMP8 + 4], CONST_1_3 ; move 1/3 to temp buffer
				movlps	xmm2, [VAR_TMP8] ; move [1/3, 1/3] to low part of xmm2
				
				mov		dword [VAR_TMP8], CONST_2_3 ; move 2/3 to temp buffer
				mov		dword [VAR_TMP8 + 4], CONST_2_3 ; move 2/3 to temp buffer
				movhps	xmm2, [VAR_TMP8]; move [2/3, 2/3] to high part of xmm2
				
				mulps	xmm1, xmm2		; multiply U and V by appropriate factors
										; xmm1 - (low) [1/3 * Ux, 1/3 * Uy, 2/3 * Vx, 2/3 * Vy] (high)
										
				movlps	xmm2, [rsi]		; move Ax, Ay to low part of xmm2
				movhps	xmm2, [rsi]		; move Ax, Ay to high part of xmm2
				
				addps	xmm1, xmm2		; add A to vectors
										; xmm1 - (low) [Px, Py, Rx, Ry] (high)
										
										; --- COMPUTE Q ---
				movaps	xmm2, xmm0		; move Ux, Uy, Vx, Vy to xmm2
				
				mov		dword [VAR_TMP8], CONST_1_2 ; move 1/2 to temp buffer
				mov		dword [VAR_TMP8 + 4], CONST_1_2 ; move 1/2 to temp buffer
				movlps	xmm3, [VAR_TMP8] ; move [1/2, 1/2] to low part of xmm3
				
				mov		dword [VAR_TMP8], CONST_SQRT3_6 ; move sqrt(3) / 6 to temp buffer
				mov		dword [VAR_TMP8 + 4], CONST_SQRT3_6 ; move sqrt(3) / 6 to temp buffer
				movhps	xmm3, [VAR_TMP8]; move [sqrt(3) / 6, sqrt(3) / 6] to high part of xmm3
				
				mulps	xmm2, xmm3		; multiply by appropriate factors
										; xmm2 - (low) [1/2 * Ux, 1/2 * Uy, (sqrt(3) / 6) * Vx, (sqrt(3) / 6) * Vy] (high)
				
				xorps	xmm3, xmm3		; zero xmm3
				movlps	xmm3, [rsi]		; move Ax, Ay to low part of xmm3
				addps	xmm2, xmm3		; add A to low vector in xmm2
				
				movhlps xmm3, xmm2		; move (sqrt(3) / 6) * Vx), (sqrt(3) / 6) * Vy) from high part of xmm2 to low part of xmm3
				addps	xmm2, xmm3		; compute final Q
										; xmm2 - (low) [Qx, Qy, ---, ---] (high)
								
										; STORE A P Q R in memory
				movlps	xmm3, [rsi]		; move Ax, Ay to low part of xmm3
				movlps	[rdi], xmm3		; store Ax, Ay in memory
				movlps	[rdi + 8], xmm1 ; store Px, Py in memory
				movlps	[rdi + 16], xmm2 ; store Qx, Qy in memory
				movhps	[rdi + 24], xmm1 ; store Rx, Ry in memory
										
				add		rsi, 8			; take next point
				add		rdi, 32			; next free chunk of memory
				
				dec		rdx				; decrement counter
				test	rdx, rdx		; test if rdx is 0
				jnz		koch_loop		; loop if rdx != 0 (there are remaining points to process)
				
				movlps	xmm3, [rsi]		; move last point to low part of xmm3
				movlps	[rdi], xmm3		; store last point in memory
				
										; --- EPILOGUE ---
				mov		rsp, rbp		; restore stack pointer
				pop		rbp				; restore frame pointer
				ret
%undef	VAR_TMP8