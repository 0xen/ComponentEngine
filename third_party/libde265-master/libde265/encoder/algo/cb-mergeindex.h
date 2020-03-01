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

#ifndef CB_MERGEINDEX_H
#define CB_MERGEINDEX_H

#include <nal-parser.h>
#include <decctx.h>
#include <slice.h>
#include <scan.h>
#include <intrapred.h>
#include <transform.h>
#include <fallback-dct.h>
#include <quality.h>
#include <fallback.h>
#include <configparam.h>

#include <encoder/algo/algo.h>
#include <encoder/algo/tb-split.h>


// ========== CB Skip/Inter decision ==========

class Algo_CB_MergeIndex : public Algo_CB
{
 public:
  Algo_CB_MergeIndex() : mCodeResidual(false) { }
  virtual ~Algo_CB_MergeIndex() { }

  void set_code_residual(bool flag=true) { mCodeResidual=flag; }

  void setChildAlgo(Algo_TB_Split* algo) { mTBSplit = algo; }
  // TODO void setInterChildAlgo(Algo_CB_IntraPartMode* algo) { mInterPartModeAlgo = algo; }

  virtual const char* name() const { return "cb-mergeindex"; }

 protected:
  Algo_TB_Split* mTBSplit;

  bool mCodeResidual;
};

class Algo_CB_MergeIndex_Fixed : public Algo_CB_MergeIndex
{
 public:
  virtual enc_cb* analyze(encoder_context*,
                          context_model_table&,
                          enc_cb* cb);
};

#endif
