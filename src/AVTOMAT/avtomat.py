#**************************************#
# ES-AVTOMAT                           #
#                                      #
# > build helper for ESPECTRO          #
#   ---     ---     ---     ---     ---#
# LIBRE SOFTWARE                       #
#                                      #
# > Licensed under GNU GPL3            #
# > be a bro and inherit               #
#   ---     ---     ---     ---     ---#
# CONTRIBUTORS                         #
#                                      #
# > lyeb,                              #
#                                      #
#**************************************#

import os, datetime;
today = datetime.datetime.now();

#   ---     ---     ---     ---     ---

pstr  =        "/*/************************************************\n";
pstr  = pstr + " * LIBRE BOILERPASTE GENERATED BY ES-AVTOMAT      *\n";
pstr  = pstr + " * LICENSED UNDER GNU GPL3 : BE A BRO AND INHERIT *\n";
pstr  = pstr +f" * COPYLEFT (c) CIXXPAKK {today.year}             *\n";
pstr  = pstr + " **************************************************/\n\n";

#   ---     ---     ---     ---     ---

print("\n>AVTOMAT"); pwd=os.getcwd();
if not os.path.exists(pwd+"/calout.h"):
    print("->generated calout.h");
    import mkcalout; mkcalout.gen(pstr, pwd);

import struct; plat=struct.calcsize("P") * 8
plat='x64' if plat==64 else 'x32'

for fold in ["KVRNEL", "MAMMOTH", "SIN", "CHASM", "ESPECTRO"]:

    dpath=os.path.abspath(pwd+f"/../../trashcan/{plat}/{fold}");
    d = dpath.split("\\"); r = d[0]; 
    for sp in d[1:]:
        r = r + f"\\{sp}";
        if not os.path.exists(r): os.makedir(r);

    if not os.path.exists(dpath+"/MKLOG"):
        print(f"->generated {fold} can");
        with open(dpath+"/MKLOG", "w+") as mkflags:
            mkflags.write("-D KVR_DEBUG=0x0D");

print("\n Ready\n");

#   ---     ---     ---     ---     ---