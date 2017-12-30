
global testasm

testasm:		
									; --- PROLOGUE ---
				push ebp
				mov ebp, esp

				movlpd xmm0, [ebp + 8]
				movhpd xmm0, [ebp + 16]
				movlpd xmm1, [ebp + 16]
				mulpd xmm0, xmm1
				mov eax, [ebp + 24]
				movlpd [eax], xmm0
				
									; --- EPILOGUE ---
				mov esp, ebp
				pop ebp
				ret