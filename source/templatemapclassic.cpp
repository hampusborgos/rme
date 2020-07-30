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

ConversionMap getReplacementMapClassic() {
	ConversionMap replacement_map;
	std::vector<uint16_t> veckey, vecval;

	vecval.clear();
	vecval.push_back(102);
	replacement_map.stm[102] = vecval;

	vecval.clear();
	vecval.push_back(105);
	replacement_map.stm[105] = vecval;

	vecval.clear();
	vecval.push_back(107);
	replacement_map.stm[107] = vecval;

	vecval.clear();
	vecval.push_back(146);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4667);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(147);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4669);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(148);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4670);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(149);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4668);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(150);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4675);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(151);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4676);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(152);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4678);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(153);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4677);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(154);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4671);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(155);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4672);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(156);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4674);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(157);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4673);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(158);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4596);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(159);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4598);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(160);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4599);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(161);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4597);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(162);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4604);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(163);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4605);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(164);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4607);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(165);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4606);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(166);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4600);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(167);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4601);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(168);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4603);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(169);
	veckey.clear();
	veckey.push_back(105);
	veckey.push_back(4602);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(219);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(220);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(221);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(222);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(223);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4550);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(224);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4551);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(225);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4553);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(226);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4552);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(227);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4546);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(228);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4547);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(229);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4549);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(230);
	veckey.clear();
	veckey.push_back(106);
	veckey.push_back(4548);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(232);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(233);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(234);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(235);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(236);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4550);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(237);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4551);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(238);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4553);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(239);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4552);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(240);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4546);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(241);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4547);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(242);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4549);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(243);
	veckey.clear();
	veckey.push_back(231);
	veckey.push_back(4548);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(296);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4599);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(297);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4597);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(298);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4599);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(299);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4596);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(300);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4598);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(301);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4596);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(302);
	veckey.clear();
	veckey.push_back(103);
	veckey.push_back(4598);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(304);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4670);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(305);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4668);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(306);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4670);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(307);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4667);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(308);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4667);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(309);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4669);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(310);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4669);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(311);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4597);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(312);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4599);
	veckey.push_back(4542);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(313);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4597);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(314);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4599);
	veckey.push_back(4544);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(315);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4596);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(316);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4598);
	veckey.push_back(4543);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(317);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4596);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(318);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4598);
	veckey.push_back(4545);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(319);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4670);
	veckey.push_back(4596);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(320);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4668);
	veckey.push_back(4596);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(321);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4670);
	veckey.push_back(4598);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(322);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4668);
	veckey.push_back(4598);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(323);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4667);
	veckey.push_back(4597);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(324);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4669);
	veckey.push_back(4597);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(325);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4667);
	veckey.push_back(4599);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	vecval.clear();
	vecval.push_back(326);
	veckey.clear();
	veckey.push_back(4566);
	veckey.push_back(4669);
	veckey.push_back(4599);
	std::sort(veckey.begin(), veckey.end());
	replacement_map.mtm[veckey] = vecval;

	return replacement_map;
}
