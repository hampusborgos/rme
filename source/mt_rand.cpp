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

static inline unsigned long int mt_get (void *vstate);
static double mt_get_double (void *vstate);
static void mt_set (void *state, unsigned long int s);

#define N 624   /* Period parameters */
#define M 397

/* most significant w-r bits */
static const unsigned long UPPER_MASK = 0x80000000UL;

/* least significant r bits */
static const unsigned long LOWER_MASK = 0x7fffffffUL;

typedef struct
  {
    unsigned long mt[N];
    int mti;
  }
mt_state_t;

static inline unsigned long
mt_get (void *vstate)
{
  mt_state_t *state = (mt_state_t *) vstate;

  unsigned long k ;
  unsigned long int *const mt = state->mt;

#define MAGIC(y) (((y)&0x1) ? 0x9908b0dfUL : 0)

  if(state->mti >= N)
    {   /* generate N words at one time */
      int kk;

      for(kk = 0; kk < N - M; kk++)
        {
          unsigned long y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
          mt[kk] = mt[kk + M] ^ (y >> 1) ^ MAGIC(y);
        }
      for(; kk < N - 1; kk++)
        {
          unsigned long y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
          mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ MAGIC(y);
        }

      {
        unsigned long y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ MAGIC(y);
      }

      state->mti = 0;
    }

  /* Tempering */

  k = mt[state->mti];
  k ^= (k >> 11);
  k ^= (k << 7) & 0x9d2c5680UL;
  k ^= (k << 15) & 0xefc60000UL;
  k ^= (k >> 18);

  state->mti++;

  return k;
}

static double
mt_get_double (void * vstate)
{
  return mt_get (vstate) / 4294967296.0 ;
}

static void
mt_set (void *vstate, unsigned long int s)
{
  mt_state_t *state = (mt_state_t *) vstate;
  int i;

  if(s == 0)
    s = 4357;   /* the default seed is 4357 */

  state->mt[0]= s & 0xffffffffUL;

  for(i = 1; i < N; i++)
    {
      /* See Knuth's "Art of Computer Programming" Vol. 2, 3rd
         Ed. p.106 for multiplier. */

      state->mt[i] =
        (1812433253UL * (state->mt[i-1] ^ (state->mt[i-1] >> 30)) + i);

      state->mt[i] &= 0xffffffffUL;
    }

  state->mti = i;
}

static mt_state_t mt_state;

void mt_seed(unsigned long s) {
	mt_set(&mt_state, s);
}

unsigned long mt_randi() {
	return mt_get(&mt_state);
}

double mt_randd() {
	return mt_get_double(&mt_state);
}
