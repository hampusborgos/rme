//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"



#include "templates.h"

ConversionMap getReplacementMapFrom800To810() {
	ConversionMap replacement_map;
	std::vector<uint16_t> veckey, vecval;

	vecval.clear();
	vecval.push_back(4526);
	replacement_map.stm[102] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	replacement_map.stm[105] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	replacement_map.stm[107] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4542);
	replacement_map.stm[110] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4544);
	replacement_map.stm[111] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4545);
	replacement_map.stm[112] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4543);
	replacement_map.stm[113] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4550);
	replacement_map.stm[114] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4551);
	replacement_map.stm[115] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4553);
	replacement_map.stm[116] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4552);
	replacement_map.stm[117] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4546);
	replacement_map.stm[118] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4547);
	replacement_map.stm[119] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4549);
	replacement_map.stm[120] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4548);
	replacement_map.stm[121] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4542);
	replacement_map.stm[122] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4544);
	replacement_map.stm[123] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4545);
	replacement_map.stm[124] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4543);
	replacement_map.stm[125] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4550);
	replacement_map.stm[126] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4551);
	replacement_map.stm[127] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4553);
	replacement_map.stm[128] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4552);
	replacement_map.stm[129] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4546);
	replacement_map.stm[130] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4547);
	replacement_map.stm[131] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4549);
	replacement_map.stm[132] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4548);
	replacement_map.stm[133] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4596);
	replacement_map.stm[134] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4598);
	replacement_map.stm[135] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4599);
	replacement_map.stm[136] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4597);
	replacement_map.stm[137] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4604);
	replacement_map.stm[138] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4605);
	replacement_map.stm[139] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4607);
	replacement_map.stm[140] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4606);
	replacement_map.stm[141] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4600);
	replacement_map.stm[142] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4601);
	replacement_map.stm[143] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4603);
	replacement_map.stm[144] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4602);
	replacement_map.stm[145] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4556);
	replacement_map.stm[146] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4554);
	replacement_map.stm[147] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4555);
	replacement_map.stm[148] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4557);
	replacement_map.stm[149] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4561);
	replacement_map.stm[150] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4560);
	replacement_map.stm[151] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4558);
	replacement_map.stm[152] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4559);
	replacement_map.stm[153] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4565);
	replacement_map.stm[154] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4564);
	replacement_map.stm[155] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4562);
	replacement_map.stm[156] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4563);
	replacement_map.stm[157] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4596);
	replacement_map.stm[158] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4598);
	replacement_map.stm[159] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4599);
	replacement_map.stm[160] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4597);
	replacement_map.stm[161] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4604);
	replacement_map.stm[162] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4605);
	replacement_map.stm[163] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4607);
	replacement_map.stm[164] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4606);
	replacement_map.stm[165] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4600);
	replacement_map.stm[166] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4601);
	replacement_map.stm[167] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4603);
	replacement_map.stm[168] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4602);
	replacement_map.stm[169] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4554);
	replacement_map.stm[170] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4556);
	replacement_map.stm[171] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4557);
	replacement_map.stm[172] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4555);
	replacement_map.stm[173] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4562);
	replacement_map.stm[174] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4563);
	replacement_map.stm[175] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4565);
	replacement_map.stm[176] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4564);
	replacement_map.stm[177] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4558);
	replacement_map.stm[178] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4559);
	replacement_map.stm[179] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4561);
	replacement_map.stm[180] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4560);
	replacement_map.stm[181] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4681);
	replacement_map.stm[182] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4679);
	replacement_map.stm[183] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4680);
	replacement_map.stm[184] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4682);
	replacement_map.stm[185] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4686);
	replacement_map.stm[186] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4685);
	replacement_map.stm[187] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4683);
	replacement_map.stm[188] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4684);
	replacement_map.stm[189] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4690);
	replacement_map.stm[190] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4689);
	replacement_map.stm[191] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4687);
	replacement_map.stm[192] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4688);
	replacement_map.stm[193] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4542);
	replacement_map.stm[195] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4544);
	replacement_map.stm[196] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4545);
	replacement_map.stm[197] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4543);
	replacement_map.stm[198] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4550);
	replacement_map.stm[199] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4551);
	replacement_map.stm[200] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4553);
	replacement_map.stm[201] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4552);
	replacement_map.stm[202] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4546);
	replacement_map.stm[203] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4547);
	replacement_map.stm[204] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4549);
	replacement_map.stm[205] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4548);
	replacement_map.stm[206] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4554);
	replacement_map.stm[207] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4556);
	replacement_map.stm[208] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4557);
	replacement_map.stm[209] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4555);
	replacement_map.stm[210] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4562);
	replacement_map.stm[211] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4563);
	replacement_map.stm[212] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4565);
	replacement_map.stm[213] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4564);
	replacement_map.stm[214] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4558);
	replacement_map.stm[215] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4559);
	replacement_map.stm[216] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4561);
	replacement_map.stm[217] = vecval;

	vecval.clear();
	vecval.push_back(194);
	vecval.push_back(4560);
	replacement_map.stm[218] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4542);
	replacement_map.stm[219] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4544);
	replacement_map.stm[220] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4545);
	replacement_map.stm[221] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4543);
	replacement_map.stm[222] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4550);
	replacement_map.stm[223] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4551);
	replacement_map.stm[224] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4553);
	replacement_map.stm[225] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4552);
	replacement_map.stm[226] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4546);
	replacement_map.stm[227] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4547);
	replacement_map.stm[228] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4549);
	replacement_map.stm[229] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4548);
	replacement_map.stm[230] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4542);
	replacement_map.stm[232] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4544);
	replacement_map.stm[233] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4545);
	replacement_map.stm[234] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4543);
	replacement_map.stm[235] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4550);
	replacement_map.stm[236] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4551);
	replacement_map.stm[237] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4553);
	replacement_map.stm[238] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4552);
	replacement_map.stm[239] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4546);
	replacement_map.stm[240] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4547);
	replacement_map.stm[241] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4549);
	replacement_map.stm[242] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4548);
	replacement_map.stm[243] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4596);
	replacement_map.stm[244] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4598);
	replacement_map.stm[245] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4599);
	replacement_map.stm[246] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4597);
	replacement_map.stm[247] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4604);
	replacement_map.stm[248] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4605);
	replacement_map.stm[249] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4607);
	replacement_map.stm[250] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4606);
	replacement_map.stm[251] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4600);
	replacement_map.stm[252] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4601);
	replacement_map.stm[253] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4603);
	replacement_map.stm[254] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4602);
	replacement_map.stm[255] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4516);
	replacement_map.stm[256] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4514);
	replacement_map.stm[257] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4515);
	replacement_map.stm[258] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4517);
	replacement_map.stm[259] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4521);
	replacement_map.stm[260] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4520);
	replacement_map.stm[261] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4518);
	replacement_map.stm[262] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4519);
	replacement_map.stm[263] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4525);
	replacement_map.stm[264] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4524);
	replacement_map.stm[265] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4522);
	replacement_map.stm[266] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4523);
	replacement_map.stm[267] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4542);
	replacement_map.stm[268] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4544);
	replacement_map.stm[269] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4545);
	replacement_map.stm[270] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4543);
	replacement_map.stm[271] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4550);
	replacement_map.stm[272] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4551);
	replacement_map.stm[273] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4553);
	replacement_map.stm[274] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4552);
	replacement_map.stm[275] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4546);
	replacement_map.stm[276] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4547);
	replacement_map.stm[277] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4549);
	replacement_map.stm[278] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4548);
	replacement_map.stm[279] = vecval;

	vecval.clear();
	vecval.push_back(4570);
	vecval.push_back(4542);
	replacement_map.stm[281] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4554);
	replacement_map.stm[282] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4555);
	replacement_map.stm[283] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4557);
	replacement_map.stm[284] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4561);
	replacement_map.stm[285] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4560);
	replacement_map.stm[286] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4558);
	replacement_map.stm[287] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4559);
	replacement_map.stm[288] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4565);
	replacement_map.stm[289] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4564);
	replacement_map.stm[290] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4562);
	replacement_map.stm[291] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4563);
	replacement_map.stm[292] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4597);
	vecval.push_back(4542);
	replacement_map.stm[295] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4599);
	vecval.push_back(4542);
	replacement_map.stm[296] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4597);
	vecval.push_back(4544);
	replacement_map.stm[297] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4599);
	vecval.push_back(4544);
	replacement_map.stm[298] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4596);
	vecval.push_back(4543);
	replacement_map.stm[299] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4598);
	vecval.push_back(4543);
	replacement_map.stm[300] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4596);
	vecval.push_back(4545);
	replacement_map.stm[301] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4598);
	vecval.push_back(4545);
	replacement_map.stm[302] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4542);
	replacement_map.stm[303] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4670);
	vecval.push_back(4542);
	replacement_map.stm[304] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4544);
	replacement_map.stm[305] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4670);
	vecval.push_back(4544);
	replacement_map.stm[306] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4543);
	replacement_map.stm[307] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4545);
	replacement_map.stm[308] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4543);
	replacement_map.stm[309] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4545);
	replacement_map.stm[310] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4597);
	vecval.push_back(4542);
	replacement_map.stm[311] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4599);
	vecval.push_back(4542);
	replacement_map.stm[312] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4597);
	vecval.push_back(4544);
	replacement_map.stm[313] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4599);
	vecval.push_back(4544);
	replacement_map.stm[314] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4596);
	vecval.push_back(4543);
	replacement_map.stm[315] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4598);
	vecval.push_back(4543);
	replacement_map.stm[316] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4596);
	vecval.push_back(4545);
	replacement_map.stm[317] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4598);
	vecval.push_back(4545);
	replacement_map.stm[318] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4670);
	vecval.push_back(4596);
	replacement_map.stm[319] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4596);
	replacement_map.stm[320] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4670);
	vecval.push_back(4598);
	replacement_map.stm[321] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4598);
	replacement_map.stm[322] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4597);
	replacement_map.stm[323] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4597);
	replacement_map.stm[324] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4599);
	replacement_map.stm[325] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4599);
	replacement_map.stm[326] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4555);
	vecval.push_back(4546);
	replacement_map.stm[327] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4557);
	vecval.push_back(4547);
	replacement_map.stm[328] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4555);
	vecval.push_back(4548);
	replacement_map.stm[329] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4557);
	vecval.push_back(4549);
	replacement_map.stm[330] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4554);
	vecval.push_back(4548);
	replacement_map.stm[331] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4554);
	vecval.push_back(4549);
	replacement_map.stm[332] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4556);
	vecval.push_back(4547);
	replacement_map.stm[333] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4556);
	vecval.push_back(4546);
	replacement_map.stm[334] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4799);
	vecval.push_back(4596);
	replacement_map.stm[335] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4596);
	replacement_map.stm[336] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4670);
	vecval.push_back(4598);
	replacement_map.stm[337] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4668);
	vecval.push_back(4598);
	replacement_map.stm[338] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4597);
	replacement_map.stm[339] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4597);
	replacement_map.stm[340] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4667);
	vecval.push_back(4599);
	replacement_map.stm[341] = vecval;

	vecval.clear();
	vecval.push_back(4566);
	vecval.push_back(4669);
	vecval.push_back(4599);
	replacement_map.stm[342] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4670);
	vecval.push_back(4554);
	replacement_map.stm[343] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4668);
	vecval.push_back(4554);
	replacement_map.stm[344] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4670);
	vecval.push_back(4556);
	replacement_map.stm[345] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4668);
	vecval.push_back(4556);
	replacement_map.stm[346] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4667);
	vecval.push_back(4557);
	replacement_map.stm[347] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4667);
	vecval.push_back(4555);
	replacement_map.stm[348] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4669);
	vecval.push_back(4557);
	replacement_map.stm[349] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4669);
	vecval.push_back(4555);
	replacement_map.stm[350] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4544);
	replacement_map.stm[393] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4542);
	replacement_map.stm[394] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4545);
	replacement_map.stm[395] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4543);
	replacement_map.stm[396] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4553);
	replacement_map.stm[397] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4552);
	replacement_map.stm[398] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4551);
	replacement_map.stm[399] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4550);
	replacement_map.stm[400] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4546);
	replacement_map.stm[401] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4547);
	replacement_map.stm[402] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4549);
	replacement_map.stm[403] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(4548);
	replacement_map.stm[404] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	replacement_map.stm[490] = vecval;

	vecval.clear();
	vecval.push_back(4609);
	replacement_map.stm[491] = vecval;

	vecval.clear();
	vecval.push_back(4610);
	replacement_map.stm[492] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4646);
	replacement_map.stm[494] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4644);
	replacement_map.stm[495] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4645);
	replacement_map.stm[496] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4647);
	replacement_map.stm[497] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4655);
	replacement_map.stm[498] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4654);
	replacement_map.stm[499] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4652);
	replacement_map.stm[500] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4653);
	replacement_map.stm[501] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4651);
	replacement_map.stm[502] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4650);
	replacement_map.stm[503] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4648);
	replacement_map.stm[504] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4649);
	replacement_map.stm[505] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4667);
	replacement_map.stm[506] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4669);
	replacement_map.stm[507] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4670);
	replacement_map.stm[508] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4668);
	replacement_map.stm[509] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4675);
	replacement_map.stm[510] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4676);
	replacement_map.stm[511] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4678);
	replacement_map.stm[512] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4677);
	replacement_map.stm[513] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4671);
	replacement_map.stm[514] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4672);
	replacement_map.stm[515] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4674);
	replacement_map.stm[516] = vecval;

	vecval.clear();
	vecval.push_back(493);
	vecval.push_back(4673);
	replacement_map.stm[517] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4808);
	replacement_map.stm[518] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4810);
	replacement_map.stm[519] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4811);
	replacement_map.stm[520] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4809);
	replacement_map.stm[521] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4816);
	replacement_map.stm[522] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4817);
	replacement_map.stm[523] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4819);
	replacement_map.stm[524] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4818);
	replacement_map.stm[525] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4812);
	replacement_map.stm[526] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4813);
	replacement_map.stm[527] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4815);
	replacement_map.stm[528] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4814);
	replacement_map.stm[529] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4715);
	replacement_map.stm[530] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4725);
	replacement_map.stm[531] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4714);
	replacement_map.stm[532] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4716);
	replacement_map.stm[533] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4720);
	replacement_map.stm[534] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4719);
	replacement_map.stm[535] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4717);
	replacement_map.stm[536] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4718);
	replacement_map.stm[537] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4724);
	replacement_map.stm[538] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4723);
	replacement_map.stm[539] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4721);
	replacement_map.stm[540] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4722);
	replacement_map.stm[541] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4667);
	replacement_map.stm[542] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4669);
	replacement_map.stm[543] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4670);
	replacement_map.stm[544] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4668);
	replacement_map.stm[545] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4675);
	replacement_map.stm[546] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4676);
	replacement_map.stm[547] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4678);
	replacement_map.stm[548] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4677);
	replacement_map.stm[549] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4671);
	replacement_map.stm[550] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4672);
	replacement_map.stm[551] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4674);
	replacement_map.stm[552] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4673);
	replacement_map.stm[553] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4554);
	replacement_map.stm[554] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4556);
	replacement_map.stm[555] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4557);
	replacement_map.stm[556] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4555);
	replacement_map.stm[557] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4562);
	replacement_map.stm[558] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4563);
	replacement_map.stm[559] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4565);
	replacement_map.stm[560] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4564);
	replacement_map.stm[561] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4558);
	replacement_map.stm[562] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4559);
	replacement_map.stm[563] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4561);
	replacement_map.stm[564] = vecval;

	vecval.clear();
	vecval.push_back(598);
	vecval.push_back(4560);
	replacement_map.stm[565] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4646);
	replacement_map.stm[566] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4644);
	replacement_map.stm[567] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4645);
	replacement_map.stm[568] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4647);
	replacement_map.stm[569] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4652);
	replacement_map.stm[570] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4654);
	replacement_map.stm[571] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4652);
	replacement_map.stm[572] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4653);
	replacement_map.stm[573] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4651);
	replacement_map.stm[574] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4650);
	replacement_map.stm[575] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4648);
	replacement_map.stm[576] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4649);
	replacement_map.stm[577] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4646);
	replacement_map.stm[578] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4644);
	replacement_map.stm[579] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4645);
	replacement_map.stm[580] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4647);
	replacement_map.stm[581] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4655);
	replacement_map.stm[582] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4654);
	replacement_map.stm[583] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4652);
	replacement_map.stm[584] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4653);
	replacement_map.stm[585] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4651);
	replacement_map.stm[586] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4650);
	replacement_map.stm[587] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4648);
	replacement_map.stm[588] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4649);
	replacement_map.stm[589] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4543);
	vecval.push_back(4646);
	replacement_map.stm[590] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4545);
	vecval.push_back(4646);
	replacement_map.stm[591] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4543);
	vecval.push_back(4644);
	replacement_map.stm[592] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4545);
	vecval.push_back(4644);
	replacement_map.stm[593] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4544);
	vecval.push_back(4645);
	replacement_map.stm[594] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4542);
	vecval.push_back(4645);
	replacement_map.stm[595] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4544);
	vecval.push_back(4647);
	replacement_map.stm[596] = vecval;

	vecval.clear();
	vecval.push_back(104);
	vecval.push_back(4542);
	vecval.push_back(4647);
	replacement_map.stm[597] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	replacement_map.stm[602] = vecval;

	vecval.clear();
	vecval.push_back(4749);
	replacement_map.stm[603] = vecval;

	vecval.clear();
	vecval.push_back(4750);
	replacement_map.stm[604] = vecval;

	vecval.clear();
	vecval.push_back(4751);
	replacement_map.stm[605] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4634);
	replacement_map.stm[606] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4632);
	replacement_map.stm[607] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4635);
	replacement_map.stm[608] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4633);
	replacement_map.stm[609] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4640);
	replacement_map.stm[610] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4641);
	replacement_map.stm[611] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4643);
	replacement_map.stm[612] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4642);
	replacement_map.stm[613] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4636);
	replacement_map.stm[614] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4637);
	replacement_map.stm[615] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4639);
	replacement_map.stm[616] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4638);
	replacement_map.stm[617] = vecval;

	vecval.clear();
	vecval.push_back(4614);
	replacement_map.stm[618] = vecval;

	vecval.clear();
	vecval.push_back(4615);
	replacement_map.stm[619] = vecval;

	vecval.clear();
	vecval.push_back(4616);
	replacement_map.stm[620] = vecval;

	vecval.clear();
	vecval.push_back(4617);
	replacement_map.stm[621] = vecval;

	vecval.clear();
	vecval.push_back(4618);
	replacement_map.stm[622] = vecval;

	vecval.clear();
	vecval.push_back(4619);
	replacement_map.stm[623] = vecval;

	vecval.clear();
	vecval.push_back(4620);
	replacement_map.stm[624] = vecval;

	vecval.clear();
	vecval.push_back(4621);
	replacement_map.stm[625] = vecval;

	vecval.clear();
	vecval.push_back(4622);
	replacement_map.stm[626] = vecval;

	vecval.clear();
	vecval.push_back(4623);
	replacement_map.stm[627] = vecval;

	vecval.clear();
	vecval.push_back(4624);
	replacement_map.stm[628] = vecval;

	vecval.clear();
	vecval.push_back(4625);
	replacement_map.stm[629] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4545);
	vecval.push_back(4657);
	replacement_map.stm[630] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4543);
	vecval.push_back(4661);
	replacement_map.stm[631] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4545);
	vecval.push_back(4656);
	replacement_map.stm[632] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4543);
	vecval.push_back(4660);
	replacement_map.stm[633] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4542);
	vecval.push_back(4659);
	replacement_map.stm[634] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4544);
	vecval.push_back(4663);
	replacement_map.stm[635] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4542);
	vecval.push_back(4658);
	replacement_map.stm[636] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4544);
	vecval.push_back(4662);
	replacement_map.stm[637] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4727);
	replacement_map.stm[638] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4713);
	replacement_map.stm[639] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4726);
	replacement_map.stm[640] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4716);
	replacement_map.stm[641] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4720);
	replacement_map.stm[642] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4719);
	replacement_map.stm[643] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4718);
	replacement_map.stm[644] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4717);
	replacement_map.stm[645] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4724);
	replacement_map.stm[646] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4723);
	replacement_map.stm[647] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4722);
	replacement_map.stm[648] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4721);
	replacement_map.stm[649] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4542);
	vecval.push_back(4716);
	replacement_map.stm[650] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4544);
	vecval.push_back(4716);
	replacement_map.stm[651] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4542);
	vecval.push_back(4714);
	replacement_map.stm[652] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4544);
	vecval.push_back(4714);
	replacement_map.stm[653] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4543);
	vecval.push_back(4713);
	replacement_map.stm[654] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4543);
	vecval.push_back(4715);
	replacement_map.stm[655] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4545);
	vecval.push_back(4713);
	replacement_map.stm[656] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4545);
	replacement_map.stm[657] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4737);
	replacement_map.stm[658] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4739);
	replacement_map.stm[659] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4740);
	replacement_map.stm[660] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4738);
	replacement_map.stm[661] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4745);
	replacement_map.stm[662] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4746);
	replacement_map.stm[663] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4747);
	replacement_map.stm[664] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4748);
	replacement_map.stm[665] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4744);
	replacement_map.stm[666] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4743);
	replacement_map.stm[667] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4742);
	replacement_map.stm[668] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4741);
	replacement_map.stm[669] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4737);
	replacement_map.stm[672] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4739);
	replacement_map.stm[673] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4738);
	replacement_map.stm[674] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4740);
	replacement_map.stm[675] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4745);
	replacement_map.stm[676] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4746);
	replacement_map.stm[677] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4747);
	replacement_map.stm[678] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4748);
	replacement_map.stm[679] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4744);
	replacement_map.stm[680] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	replacement_map.stm[681] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	replacement_map.stm[682] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(4741);
	replacement_map.stm[683] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4737);
	replacement_map.stm[684] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4739);
	replacement_map.stm[685] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4740);
	replacement_map.stm[686] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4738);
	replacement_map.stm[687] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4745);
	replacement_map.stm[688] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4746);
	replacement_map.stm[689] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4747);
	replacement_map.stm[690] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4748);
	replacement_map.stm[691] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4744);
	replacement_map.stm[692] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4743);
	replacement_map.stm[693] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4742);
	replacement_map.stm[694] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4741);
	replacement_map.stm[695] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4556);
	replacement_map.stm[696] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4554);
	replacement_map.stm[697] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4555);
	replacement_map.stm[698] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4557);
	replacement_map.stm[699] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4565);
	replacement_map.stm[700] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4564);
	replacement_map.stm[701] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4563);
	replacement_map.stm[702] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4562);
	replacement_map.stm[703] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4561);
	replacement_map.stm[704] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4560);
	replacement_map.stm[705] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4559);
	replacement_map.stm[706] = vecval;

	vecval.clear();
	vecval.push_back(6683);
	vecval.push_back(4558);
	replacement_map.stm[707] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4762);
	replacement_map.stm[712] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4760);
	replacement_map.stm[713] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4761);
	replacement_map.stm[714] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4763);
	replacement_map.stm[715] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4771);
	replacement_map.stm[716] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4770);
	replacement_map.stm[717] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4769);
	replacement_map.stm[718] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4768);
	replacement_map.stm[719] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4767);
	replacement_map.stm[720] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4766);
	replacement_map.stm[721] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4765);
	replacement_map.stm[722] = vecval;

	vecval.clear();
	vecval.push_back(708);
	vecval.push_back(4764);
	replacement_map.stm[723] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4667);
	replacement_map.stm[725] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4669);
	replacement_map.stm[726] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4670);
	replacement_map.stm[727] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4668);
	replacement_map.stm[728] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4675);
	replacement_map.stm[729] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4676);
	replacement_map.stm[730] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4678);
	replacement_map.stm[731] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4677);
	replacement_map.stm[732] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4671);
	replacement_map.stm[733] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4672);
	replacement_map.stm[734] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4674);
	replacement_map.stm[735] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4673);
	replacement_map.stm[736] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4668);
	vecval.push_back(4542);
	replacement_map.stm[737] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4670);
	vecval.push_back(4542);
	replacement_map.stm[738] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4668);
	vecval.push_back(4544);
	replacement_map.stm[739] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4670);
	vecval.push_back(4544);
	replacement_map.stm[740] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4667);
	replacement_map.stm[741] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4667);
	vecval.push_back(4545);
	replacement_map.stm[742] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4669);
	vecval.push_back(4543);
	replacement_map.stm[743] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4669);
	vecval.push_back(4545);
	replacement_map.stm[744] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4597);
	vecval.push_back(4542);
	replacement_map.stm[745] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4599);
	vecval.push_back(4542);
	replacement_map.stm[746] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4597);
	vecval.push_back(4544);
	replacement_map.stm[747] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4599);
	vecval.push_back(4544);
	replacement_map.stm[748] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4596);
	vecval.push_back(4543);
	replacement_map.stm[749] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4598);
	vecval.push_back(4543);
	replacement_map.stm[750] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4596);
	vecval.push_back(4545);
	replacement_map.stm[751] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4598);
	vecval.push_back(4545);
	replacement_map.stm[752] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4542);
	replacement_map.stm[753] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4544);
	replacement_map.stm[754] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4545);
	replacement_map.stm[755] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4543);
	replacement_map.stm[756] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4550);
	replacement_map.stm[757] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4551);
	replacement_map.stm[758] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4553);
	replacement_map.stm[759] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4552);
	replacement_map.stm[760] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4546);
	replacement_map.stm[761] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4547);
	replacement_map.stm[762] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4549);
	vecval.push_back(4549);
	replacement_map.stm[763] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4548);
	replacement_map.stm[764] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4596);
	replacement_map.stm[765] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4598);
	replacement_map.stm[766] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4599);
	replacement_map.stm[767] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4597);
	replacement_map.stm[768] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4604);
	replacement_map.stm[769] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4605);
	replacement_map.stm[770] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4607);
	replacement_map.stm[771] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4606);
	replacement_map.stm[772] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4600);
	replacement_map.stm[773] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4601);
	replacement_map.stm[774] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4603);
	replacement_map.stm[775] = vecval;

	vecval.clear();
	vecval.push_back(724);
	vecval.push_back(4602);
	replacement_map.stm[776] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4784);
	replacement_map.stm[792] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4786);
	replacement_map.stm[793] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4787);
	replacement_map.stm[794] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4785);
	replacement_map.stm[795] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4792);
	replacement_map.stm[796] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4793);
	replacement_map.stm[797] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4795);
	replacement_map.stm[798] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4794);
	replacement_map.stm[799] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4788);
	replacement_map.stm[800] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4789);
	replacement_map.stm[801] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4791);
	replacement_map.stm[802] = vecval;

	vecval.clear();
	vecval.push_back(791);
	vecval.push_back(4790);
	replacement_map.stm[803] = vecval;

	vecval.clear();
	vecval.push_back(4832);
	vecval.push_back(4545);
	replacement_map.stm[805] = vecval;

	vecval.clear();
	vecval.push_back(4833);
	vecval.push_back(4543);
	replacement_map.stm[807] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4681);
	vecval.push_back(4543);
	replacement_map.stm[808] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4681);
	vecval.push_back(4545);
	replacement_map.stm[809] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4679);
	vecval.push_back(4543);
	replacement_map.stm[810] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4679);
	vecval.push_back(4545);
	replacement_map.stm[811] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4682);
	vecval.push_back(4542);
	replacement_map.stm[812] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4682);
	vecval.push_back(4544);
	replacement_map.stm[813] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4680);
	vecval.push_back(4542);
	replacement_map.stm[814] = vecval;

	vecval.clear();
	vecval.push_back(106);
	vecval.push_back(4680);
	vecval.push_back(4544);
	replacement_map.stm[815] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4596);
	replacement_map.stm[816] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4598);
	replacement_map.stm[817] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4599);
	replacement_map.stm[818] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4597);
	replacement_map.stm[819] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4604);
	replacement_map.stm[820] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4605);
	replacement_map.stm[821] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4607);
	replacement_map.stm[822] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4606);
	replacement_map.stm[823] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4600);
	replacement_map.stm[824] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4601);
	replacement_map.stm[825] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4603);
	replacement_map.stm[826] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4602);
	replacement_map.stm[827] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4597);
	vecval.push_back(4542);
	replacement_map.stm[828] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4599);
	vecval.push_back(4542);
	replacement_map.stm[829] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4597);
	vecval.push_back(4544);
	replacement_map.stm[830] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4599);
	vecval.push_back(4544);
	replacement_map.stm[831] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4596);
	vecval.push_back(4543);
	replacement_map.stm[832] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4598);
	vecval.push_back(4543);
	replacement_map.stm[833] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4596);
	vecval.push_back(4545);
	replacement_map.stm[834] = vecval;

	vecval.clear();
	vecval.push_back(231);
	vecval.push_back(4598);
	vecval.push_back(4545);
	replacement_map.stm[835] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4760);
	replacement_map.stm[837] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4762);
	replacement_map.stm[838] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4763);
	replacement_map.stm[839] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4761);
	replacement_map.stm[840] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4764);
	replacement_map.stm[841] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4765);
	replacement_map.stm[842] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4766);
	replacement_map.stm[843] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4767);
	replacement_map.stm[844] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4768);
	replacement_map.stm[845] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4769);
	replacement_map.stm[846] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4771);
	replacement_map.stm[847] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4770);
	replacement_map.stm[848] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4542);
	replacement_map.stm[849] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4544);
	replacement_map.stm[850] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4545);
	replacement_map.stm[851] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4543);
	replacement_map.stm[852] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4546);
	replacement_map.stm[853] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4547);
	replacement_map.stm[854] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4548);
	replacement_map.stm[855] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4549);
	replacement_map.stm[856] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4550);
	replacement_map.stm[857] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4551);
	replacement_map.stm[858] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4553);
	replacement_map.stm[859] = vecval;

	vecval.clear();
	vecval.push_back(836);
	vecval.push_back(4552);
	replacement_map.stm[860] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4772);
	replacement_map.stm[861] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4773);
	replacement_map.stm[862] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4774);
	replacement_map.stm[863] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4775);
	replacement_map.stm[864] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4776);
	replacement_map.stm[865] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4777);
	replacement_map.stm[866] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4778);
	replacement_map.stm[867] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4779);
	replacement_map.stm[868] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4768);
	replacement_map.stm[869] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4781);
	replacement_map.stm[870] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4783);
	replacement_map.stm[871] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	vecval.push_back(4782);
	replacement_map.stm[872] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(893);
	replacement_map.stm[879] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(891);
	replacement_map.stm[880] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(892);
	replacement_map.stm[881] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(894);
	replacement_map.stm[882] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(897);
	replacement_map.stm[883] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(898);
	replacement_map.stm[884] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(895);
	replacement_map.stm[885] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(896);
	replacement_map.stm[886] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(901);
	replacement_map.stm[887] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(902);
	replacement_map.stm[888] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(899);
	replacement_map.stm[889] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	vecval.push_back(900);
	replacement_map.stm[890] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4556);
	replacement_map.stm[3246] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4554);
	replacement_map.stm[3247] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4557);
	replacement_map.stm[3248] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4557);
	replacement_map.stm[3249] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4561);
	replacement_map.stm[3250] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4560);
	replacement_map.stm[3251] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4559);
	replacement_map.stm[3252] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4558);
	replacement_map.stm[3253] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4565);
	replacement_map.stm[3254] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4564);
	replacement_map.stm[3255] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4563);
	replacement_map.stm[3256] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(4562);
	replacement_map.stm[3257] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	replacement_map.stm[3258] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3305);
	replacement_map.stm[3259] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3304);
	replacement_map.stm[3260] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3303);
	replacement_map.stm[3261] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3302);
	replacement_map.stm[3262] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3305);
	replacement_map.stm[3266] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3304);
	replacement_map.stm[3267] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3303);
	replacement_map.stm[3268] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3302);
	replacement_map.stm[3269] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3301);
	replacement_map.stm[3270] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3300);
	replacement_map.stm[3271] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3299);
	replacement_map.stm[3272] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3298);
	replacement_map.stm[3273] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3306);
	replacement_map.stm[3274] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3307);
	replacement_map.stm[3275] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3308);
	replacement_map.stm[3276] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3309);
	replacement_map.stm[3277] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3301);
	replacement_map.stm[3278] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3300);
	replacement_map.stm[3279] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3299);
	replacement_map.stm[3280] = vecval;

	vecval.clear();
	vecval.push_back(103);
	vecval.push_back(3298);
	replacement_map.stm[3281] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3282] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3283] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3284] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3285] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3286] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3287] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3288] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3289] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3290] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3291] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3292] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3293] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3294] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3295] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3296] = vecval;

	vecval.clear();
	vecval.push_back(3263);
	replacement_map.stm[3297] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3305);
	replacement_map.stm[3553] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3304);
	replacement_map.stm[3554] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3303);
	replacement_map.stm[3555] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3302);
	replacement_map.stm[3556] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3298);
	replacement_map.stm[3557] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3306);
	replacement_map.stm[3558] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3299);
	replacement_map.stm[3559] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3307);
	replacement_map.stm[3560] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3308);
	replacement_map.stm[3561] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3300);
	replacement_map.stm[3562] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3309);
	replacement_map.stm[3563] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3301);
	replacement_map.stm[3564] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3298);
	replacement_map.stm[3565] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3300);
	replacement_map.stm[3566] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3299);
	replacement_map.stm[3567] = vecval;

	vecval.clear();
	vecval.push_back(4608);
	vecval.push_back(3301);
	replacement_map.stm[3568] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3305);
	replacement_map.stm[3575] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3304);
	replacement_map.stm[3576] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3303);
	replacement_map.stm[3577] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3302);
	replacement_map.stm[3578] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3298);
	replacement_map.stm[3579] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3306);
	replacement_map.stm[3580] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3299);
	replacement_map.stm[3581] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3307);
	replacement_map.stm[3582] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3308);
	replacement_map.stm[3583] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3300);
	replacement_map.stm[3584] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3309);
	replacement_map.stm[3585] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	vecval.push_back(3301);
	replacement_map.stm[3586] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4422] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4423] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4424] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4425] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4426] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4427] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4428] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4429] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4430] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4431] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4432] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4433] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4434] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4435] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4436] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4437] = vecval;

	vecval.clear();
	vecval.push_back(918);
	replacement_map.stm[4438] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4439] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4440] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4441] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4442] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4443] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4444] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4445] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4446] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4447] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4448] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4449] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4450] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4451] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4452] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4453] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4454] = vecval;

	vecval.clear();
	vecval.push_back(919);
	replacement_map.stm[4455] = vecval;

	vecval.clear();
	vecval.push_back(4526);
	replacement_map.stm[4567] = vecval;

	vecval.clear();
	vecval.push_back(4527);
	replacement_map.stm[4568] = vecval;

	vecval.clear();
	vecval.push_back(4528);
	replacement_map.stm[4569] = vecval;

	vecval.clear();
	vecval.push_back(4529);
	replacement_map.stm[4756] = vecval;

	vecval.clear();
	vecval.push_back(4405);
	replacement_map.stm[4757] = vecval;

	vecval.clear();
	vecval.push_back(4691);
	replacement_map.stm[4758] = vecval;

	vecval.clear();
	vecval.push_back(4576);
	replacement_map.stm[4759] = vecval;

	return replacement_map;
}
