#!/bin/bash
gcc -dD -E $PAREP_MPI_EMPI_PATH/include/mpi.h > declarations.h
gcc -dD -E $PAREP_MPI_EMPI_PATH/include/mpi.h > typedefs.h
sed -i".bak" '/^#/d' typedefs.h
sed -i".bak" '/^$/d' typedefs.h
awk     '/{/    {while (!(/}/)) {getline X; $0 = $0 X}
                 sub (/{[       ]*/,"{")
                 sub (/[        ]*}/, "}")
                }
         1
        ' typedefs.h > temp
awk '{printf "%s%s",$0,(/;$/?"\n":" ")}' temp > typedefs.h
rm temp
sed -i".bak" '/typedef\|\<enum\>/!d' typedefs.h
sed -i".bak" '/MPI/!d' typedefs.h
sed -i".bak" 's/MPI/EMPI/g' typedefs.h
sed -i".bak" '/#[du]/!d' declarations.h
sed -i".bak" '/MPI/!d' declarations.h
sed -i".bak" 's/MPI/EMPI/g' declarations.h

sed -i '1s/^/#define _EMPI_DECLARATION_H_\n/' declarations.h
sed -i '1s/^/#ifndef _EMPI_DECLARATION_H_\n/' declarations.h
echo "#endif" >> declarations.h

sed -i '1s/^/#define _ETYPE_DEFINITION_H_\n/' typedefs.h
sed -i '1s/^/#ifndef _ETYPE_DEFINITION_H_\n/' typedefs.h
echo "#endif" >> typedefs.h
