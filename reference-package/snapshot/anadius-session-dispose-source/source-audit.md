# B32 normal kaynak/kanıt denetimi

- 0x180026880: 26899 count snapshot; 268a2..268ba liveoffset/mapsize/map and cachedstring; 268c8..268f3 capacity+1/alignment/header/sizedfree; 268f8..2690f partialreset/livecountreload; 26915 offsetresetonlynonempty; 26923 cachedRBX and signedJLE/JG; 26930 live map per block; 2693f blocksize32; 26952 live finalsize/map; 26959 shift3 modulo64; 26988/26990 mapclearafterfree
- 0x180026570: 26584 actual026880 atCONTROL+d8; 26589..265a0 load/null/free16proxy; 265a5 liveinputcapacity; 265b2..265dd alignedinputfree; 265e6..265fc inputprefixreset; 26603 actual024db0 CONTROL+20; 26608 liveCONTROL+18selfweak; 26616 weakXADD; 26623 lastweakslot08call no slotclear
- 0x180017420: 17420 nullguard; 17425 livevtable; 17428 flags1; 1742d slot10taildispatch
- 0x180026290: 26296/262a0 restore084750; 262a3 flagslowbit; 262a8 sized1c8; 262ad free; 262b2 cachedidentityreturn

Native classlayout/free provenance/EH/quiescence kapanışı ileri sürülmez.
