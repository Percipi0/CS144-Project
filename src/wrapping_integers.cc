#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

const uint64_t REL_SEQ_MAX = (uint64_t)1 << 32;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return ( zero_point + (uint32_t)n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{

  uint64_t temp = ( this->raw_value_ > zero_point.raw_value_ )
                    ? ( this->raw_value_ - zero_point.raw_value_ )
                    : ( REL_SEQ_MAX + this->raw_value_ - zero_point.raw_value_ );

  uint64_t cand_1 = ( ( checkpoint >> 32 ) << 32 ) + temp;
  uint64_t cand_2 = cand_1 - REL_SEQ_MAX;
  uint64_t cand_3 = cand_1 + REL_SEQ_MAX;

  uint64_t diff_1 = ( cand_1 > checkpoint ) ? cand_1 - checkpoint : checkpoint - cand_1;
  uint64_t diff_2 = ( cand_2 > checkpoint ) ? cand_2 - checkpoint : checkpoint - cand_2;
  uint64_t diff_3 = ( cand_3 > checkpoint ) ? cand_3 - checkpoint : checkpoint - cand_3;

  if ( diff_1 <= diff_2 && diff_1 <= diff_3 )
    return cand_1;
  if ( diff_2 <= diff_1 && diff_2 <= diff_3 )
    return cand_2;
  return cand_3;
}