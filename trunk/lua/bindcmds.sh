#!/bin/sh

echo '#include "commands.c"' \
        | ${CPP} -DUACPP -dD - \
	| sed 's/^UA\(....\)(\(.*\)).*$/\1,\2/g' \
	| ${AWK} -F ',' '{
		if ((inalia == 1) && ($1 != "ALIA")) {
			inalia = 0;
			indesc = 1;
			descs = 0;
		}
		if ((indesc == 1) && ($1 != "DESC")) {
			indesc = 0;
			inargs = 1;
		}
		if ((inargs == 1) && ($1 != "AREQ") && ($1 != "AOPT") && ($1 != "WHER")) {
			inargs = 0;
			printf("UAFUNC(%s, %s, %s)\n", funcn, arg1, funcwhere);
		}

		if ($1 == "FUNC") {
			funcn = $2;
			minarg = 0;
			maxarg = 0;
			arg1 = "0";
			funcwhere = "ANYWHERE";
			inalia = 1;
		} else if ($1 == "WHER")
			funcwhere = $2;
		else if ($1 == "DESC")
			descs++;
		else if ($1 == "AREQ") {
			if ($2 == "string")
				atype = "s";
			else if ($2 == "int")
				atype = "i";
			else if ($2 == "bool")
				atype = "b";
			else if ($2 == "window")
				atype = "W";
			else if ($2 == "buddy")
				atype = "B";
			else if ($2 == "account")
				atype = "A";
			else if ($2 == "cmember")
				atype = "M";
			else if ($2 == "idiot")
				atype = "I";
			else if ($2 == "chat")
				atype = "C";
			else if ($2 == "filename")
				atype = "F";
			else if ($2 == "varname")
				atype = "V";
			else if ($2 == "entity")
				atype = "E";
			else
				atype = "?";
			if (arg1 == "0")
				arg1 = atype;
			minarg++;
		} else if ($1 == "AOPT") {
			if ($2 == "string")
				atype = "s";
			else if ($2 == "int")
				atype = "i";
			else if ($2 == "bool")
				atype = "b";
			else if ($2 == "window")
				atype = "W";
			else if ($2 == "buddy")
				atype = "B";
			else if ($2 == "account")
				atype = "A";
			else if ($2 == "cmember")
				atype = "M";
			else if ($2 == "idiot")
				atype = "I";
			else if ($2 == "chat")
				atype = "C";
			else if ($2 == "filename")
				atype = "F";
			else if ($2 == "varname")
				atype = "V";
			else if ($2 == "entity")
				atype = "E";
			else
				atype = "?";
			if (arg1 == "0")
				arg1 = atype;
			maxarg++;
		}
	}'
