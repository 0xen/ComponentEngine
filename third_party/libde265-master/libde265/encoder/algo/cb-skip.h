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

#ifndef CB_SKIP_H
#define CB_SKIP_H

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
#include <encoder/algo/cb-mergeindex.h>


// ========== CB Skip/Inter decision ==========

class Algo_CB_Skip : public Algo_CB
{
 public:
  virtual ~Algo_CB_Skip() { }

  void setSkipAlgo(Algo_CB_MergeIndex* algo) {
    mSkipAlgo = algo;
    mSkipAlgo->set_code_residual(false);
  }

  void setNonSkipAlgo(Algo_CB* algo) { mNonSkipAlgo = algo; }

  const char* name() const { return "cb-skip"; }

 protected:
  Algo_CB_MergeIndex* mSkipAlgo;
  Algo_CB*            mNonSkipAlgo;
};

class Algo_CB_Skip_BruteForce : public Algo_CB_Skip
{
 public:
  virtual enc_cb* analyze(encoder_context*,
                          context_model_table&,
                          enc_cb* cb);

  const char* name() const { return "cb-skip-bruteforce"; }
};

#endif
