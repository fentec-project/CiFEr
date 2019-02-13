import os
import sys

deltext="rm"
copytext="cp"


def replace(namefile,oldtext,newtext):
	f = open(namefile,'r')
	filedata = f.read()
	f.close()

	newdata = filedata.replace(oldtext,newtext)

	f = open(namefile,'w')
	f.write(newdata)
	f.close()

def curveset(tb,tf,tc,nb,base,nbt,m8,mt,ct,pf,stw,sx,ab,cs) :
	bd=tb+"_"+base
	fnameh="config_big_"+bd+".h"
	os.system(copytext+" config_big.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"@NB@",nb)
	replace(fnameh,"@BASE@",base)

	fnameh="config_field_"+tf+".h"
	os.system(copytext+" config_field.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"@NBT@",nbt)
	replace(fnameh,"@M8@",m8)
	replace(fnameh,"@MT@",mt)

	ib=int(base)
	inb=int(nb)
	inbt=int(nbt)
	sh=ib*(1+((8*inb-1)//ib))-inbt
	if sh > 30 :
		sh=30
	replace(fnameh,"@SH@",str(sh))

	fnameh="config_curve_"+tc+".h"
	os.system(copytext+" config_curve.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"@CT@",ct)
	replace(fnameh,"@PF@",pf)

	replace(fnameh,"@ST@",stw)
	replace(fnameh,"@SX@",sx)
	replace(fnameh,"@CS@",cs)
	replace(fnameh,"@AB@",ab)


	fnamec="big_"+bd+".c"
	fnameh="big_"+bd+".h"

	os.system(copytext+" big.c "+fnamec)
	os.system(copytext+" big.h "+fnameh)

	replace(fnamec,"XXX",bd)
	replace(fnameh,"XXX",bd)
	os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

	fnamec="fp_"+tf+".c"
	fnameh="fp_"+tf+".h"

	os.system(copytext+" fp.c "+fnamec)
	os.system(copytext+" fp.h "+fnameh)

	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

	os.system("gcc -O3 -fPIC -std=c99 -c rom_field_"+tf+".c");

	fnamec="ecp_"+tc+".c"
	fnameh="ecp_"+tc+".h"

	os.system(copytext+" ecp.c "+fnamec)
	os.system(copytext+" ecp.h "+fnameh)

	replace(fnamec,"ZZZ",tc)
	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

	fnamec="ecdh_"+tc+".c"
	fnameh="ecdh_"+tc+".h"

	os.system(copytext+" ecdh.c "+fnamec)
	os.system(copytext+" ecdh.h "+fnameh)

	replace(fnamec,"ZZZ",tc)
	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

	os.system("gcc -O3 -fPIC -std=c99 -c rom_curve_"+tc+".c");

	if pf != "NOT" :
		fnamec="fp2_"+tf+".c"
		fnameh="fp2_"+tf+".h"

		os.system(copytext+" fp2.c "+fnamec)
		os.system(copytext+" fp2.h "+fnameh)
		replace(fnamec,"YYY",tf)
		replace(fnamec,"XXX",bd)
		replace(fnameh,"YYY",tf)
		replace(fnameh,"XXX",bd)
		os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

		fnamec="fp4_"+tf+".c"
		fnameh="fp4_"+tf+".h"

		os.system(copytext+" fp4.c "+fnamec)
		os.system(copytext+" fp4.h "+fnameh)
		replace(fnamec,"YYY",tf)
		replace(fnamec,"XXX",bd)
		replace(fnamec,"ZZZ",tc)
		replace(fnameh,"YYY",tf)
		replace(fnameh,"XXX",bd)
		replace(fnameh,"ZZZ",tc)
		os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

		if cs == "128" :
			fnamec="fp12_"+tf+".c"
			fnameh="fp12_"+tf+".h"

			os.system(copytext+" fp12.c "+fnamec)
			os.system(copytext+" fp12.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnamec,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

			fnamec="ecp2_"+tc+".c"
			fnameh="ecp2_"+tc+".h"

			os.system(copytext+" ecp2.c "+fnamec)
			os.system(copytext+" ecp2.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

			fnamec="pair_"+tc+".c"
			fnameh="pair_"+tc+".h"

			os.system(copytext+" pair.c "+fnamec)
			os.system(copytext+" pair.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

			fnamec="mpin_"+tc+".c"
			fnameh="mpin_"+tc+".h"

			os.system(copytext+" mpin.c "+fnamec)
			os.system(copytext+" mpin.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

			fnamec="bls_"+tc+".c"
			fnameh="bls_"+tc+".h"

			os.system(copytext+" bls.c "+fnamec)
			os.system(copytext+" bls.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("gcc -O3 -fPIC -std=c99 -c "+fnamec)

replace("arch.h","@WL@","64")


curveset("256","BN254","BN254","32","56","254","3","NOT_SPECIAL","WEIERSTRASS","BN","D_TYPE","NEGATIVEX","66","128")


os.system(deltext+" big.*")
os.system(deltext+" fp.*")
os.system(deltext+" ecp.*")
os.system(deltext+" ecdh.*")
os.system(deltext+" ff.*")
os.system(deltext+" rsa.*")
os.system(deltext+" config_big.h")
os.system(deltext+" config_field.h")
os.system(deltext+" config_curve.h")
os.system(deltext+" config_ff.h")
os.system(deltext+" fp2.*")
os.system(deltext+" fp4.*")
os.system(deltext+" fp8.*")
os.system(deltext+" fp16.*")

os.system(deltext+" fp12.*")
os.system(deltext+" fp24.*")
os.system(deltext+" fp48.*")

os.system(deltext+" ecp2.*")
os.system(deltext+" ecp4.*")
os.system(deltext+" ecp8.*")

os.system(deltext+" pair.*")
os.system(deltext+" mpin.*")
os.system(deltext+" bls.*")

os.system(deltext+" pair192.*")
os.system(deltext+" mpin192.*")
os.system(deltext+" bls192.*")

os.system(deltext+" pair256.*")
os.system(deltext+" mpin256.*")
os.system(deltext+" bls256.*")

# create library
os.system("gcc -O3 -fPIC -std=c99 -c pbc_support.c")

os.system("gcc -O3 -fPIC -std=c99 -c hash.c")
os.system("gcc -O3 -fPIC -std=c99 -c rand.c")
os.system("gcc -O3 -fPIC -std=c99 -c oct.c")
os.system("gcc -O3 -fPIC -std=c99 -c aes.c")
os.system("gcc -O3 -fPIC -std=c99 -c gcm.c")
os.system("gcc -O3 -fPIC -std=c99 -c newhope.c")

os.system("ar rc amcl.a *.o")

os.system(deltext+" *.o")
