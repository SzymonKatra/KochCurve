
global testasm

testasm:		
									; --- PROLOGUE ---
				push ebp
				mov ebp, esp

				movsd xmm0, [ebp + 8]
				movsd xmm1, [ebp + 16]
				cmpsd xmm0, xmm1, 5
				movd eax, xmm0
				
									; --- EPILOGUE ---
				mov esp, ebp
				pop ebp
				ret