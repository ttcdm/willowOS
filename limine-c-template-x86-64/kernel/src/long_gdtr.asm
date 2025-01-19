gdtr DW 0 ; For limit storage
     DQ 0 ; For base storage
[global setGdt]
setGdt:
   MOV   [gdtr], DI
   MOV   [gdtr+2], RSI
   LGDT  [gdtr]
   RET
