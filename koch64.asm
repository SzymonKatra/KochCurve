
global koch

default rel

section .data

vec_pr:			dd		0.3333333		; vector with constant
				dd		0.3333333		; values used to
				dd		0.6666667		; compute P an R points
				dd		0.6666667		; [1/3, 1/3, 2/3, 2/3]
				
vec_q:			dd		0.5				; vector with constant
				dd		0.5				; values used to
				dd 		0.2886751		; compute Q point
				dd		0.2886751		; [1/2, 1/2, sqrt(3) / 6, sqrt(3) / 6]

section .text

; rdi - pointer to output buffer
; rsi - pointer to input buffer
; rdx - input points count
koch:		
										; --- PROLOGUE ---
				push	rbp				; preserve frame pointer
				mov		rbp, rsp		; set up new frame pointer
				
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
				mov		eax, [rsi]		; store temporarily Ax in eax
				sal		rax, 32			; shift Ax to high part of rax
				mov		ecx, [rsi + 12] ; store temporarily By in ecx
				or		rax, rcx		; high part of rax contains now Ax, and low part contains By
				movq	xmm0, rax		; store temporarily By, Ax in low part of xmm0
				movlhps	xmm0, xmm0		; move By, Ax to high part of xmm0
										
				movlps	xmm0, [rsi + 8] ; move Bx, By to low part of xmm0
				
				movlps	xmm1, [rsi]		; move Ax, Ay to low part of xmm1
				movhps	xmm1, [rsi + 4]	; move Ay, Bx to high part of xmm1
				
				subps	xmm0, xmm1		; compute U and V values
										; xmm0 - (low) [Ux, Uy, Vx, Vy] (high)
										
										; --- COMPUTE P AND R ---
				movaps	xmm1, xmm0		; move Ux, Uy to xmm1 (moved also high part but we don't care)
				movlhps	xmm1, xmm0		; move Ux, Uy to high part of xmm1
				
				movups	xmm2, [vec_pr]	; move [1/3, 1/3, 2/3, 2/3] to xmm2
				
				mulps	xmm1, xmm2		; multiply U and V by appropriate factors
										; xmm1 - (low) [1/3 * Ux, 1/3 * Uy, 2/3 * Vx, 2/3 * Vy] (high)
										
				movlps	xmm2, [rsi]		; move Ax, Ay to low part of xmm2
				movhps	xmm2, [rsi]		; move Ax, Ay to high part of xmm2
				
				addps	xmm1, xmm2		; add A to vectors
										; xmm1 - (low) [Px, Py, Rx, Ry] (high)
										
										; --- COMPUTE Q ---
				movaps	xmm2, xmm0		; move Ux, Uy, Vx, Vy to xmm2
				
				movups	xmm3, [vec_q]	; move [1/2, 1/2, sqrt(3) / 6, sqrt(3) / 6] to xmm3
				
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