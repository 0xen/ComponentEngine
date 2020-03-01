/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TB_TRANSFORM_H
#define TB_TRANSFORM_H

#include <nal-parser.h>
#include <decctx.h>
#include <encoder/encoder-types.h>
#include <encoder/algo/algo.h>
#include <slice.h>
#include <scan.h>
#include <intrapred.h>
#include <transform.h>
#include <fallback-dct.h>
#include <quality.h>
#include <fallback.h>
#include <configparam.h>

#include <encoder/algo/tb-intrapredmode.h>
#include <encoder/algo/tb-rateestim.h>


void diff_blk(int16_t* out,int out_stride,
              const uint8_t* a_ptr, int a_stride,
              const uint8_t* b_ptr, int b_stride,
              int blkSize);


// ========== TB split decision ==========

class Algo_TB_Residual : public Algo
{
public:
  Algo_TB_Residual() { }

  virtual enc_tb* analyze(encoder_context*,
                          context_model_table&,
                          const de265_image* input,
                          enc_tb* tb,
                          int TrafoDepth, int MaxTrafoDepth, int IntraSplitFlag) = 0;

  const char* name() const { return "residual-unknown"; }
};


class Algo_TB_Transform : public Algo_TB_Residual
{
public:
  Algo_TB_Transform() : mAlgo_TB_RateEstimation(NULL) { }

  virtual enc_tb* analyze(encoder_context*,
                          context_model_table&,
                          const de265_image* input,
                          enc_tb* parent,
                          int TrafoDepth, int MaxTrafoDepth, int IntraSplitFlag);

  void setAlgo_TB_RateEstimation(Algo_TB_RateEstimation* algo) { mAlgo_TB_RateEstimation=algo; }

  const char* name() const { return "residual-FDCT"; }

 protected:
  Algo_TB_RateEstimation* mAlgo_TB_RateEstimation;
};


#endif
