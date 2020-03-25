#pragma once
#include "FixedPointType.h"
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace flttofix;
using namespace llvm;
using namespace taffo;

// value taken from Elementary Function Chapter 7. The CORDIC Algorithm
namespace TaffoMathConstant {

constexpr int TABLELENGHT = 64;
const double arctan_2power[TABLELENGHT] = {
    0.7853981633974483,     0.4636476090008061,     0.24497866312686414,
    0.12435499454676144,    0.06241880999595735,    0.031239833430268277,
    0.015623728620476831,   0.007812341060101111,   0.0039062301319669718,
    0.0019531225164788188,  0.0009765621895593195,  0.0004882812111948983,
    0.00024414062014936177, 0.00012207031189367021, 6.103515617420877e-05,
    3.0517578115526096e-05, 1.5258789061315762e-05, 7.62939453110197e-06,
    3.814697265606496e-06,  1.907348632810187e-06,  9.536743164059608e-07,
    4.7683715820308884e-07, 2.3841857910155797e-07, 1.1920928955078068e-07,
    5.960464477539055e-08,  2.9802322387695303e-08, 1.4901161193847655e-08,
    7.450580596923828e-09,  3.725290298461914e-09,  1.862645149230957e-09,
    9.313225746154785e-10,  4.656612873077393e-10,  2.3283064365386963e-10,
    1.1641532182693481e-10, 5.820766091346741e-11,  2.9103830456733704e-11,
    1.4551915228366852e-11, 7.275957614183426e-12,  3.637978807091713e-12,
    1.8189894035458565e-12, 9.094947017729282e-13,  4.547473508864641e-13,
    2.2737367544323206e-13, 1.1368683772161603e-13, 5.684341886080802e-14,
    2.842170943040401e-14,  1.4210854715202004e-14, 7.105427357601002e-15,
    3.552713678800501e-15,  1.7763568394002505e-15, 8.881784197001252e-16,
    4.440892098500626e-16,  2.220446049250313e-16,  1.1102230246251565e-16,
    5.551115123125783e-17,  2.7755575615628914e-17, 1.3877787807814457e-17,
    6.938893903907228e-18,  3.469446951953614e-18,  1.734723475976807e-18,
    8.673617379884035e-19,  4.336808689942018e-19,  2.168404344971009e-19,
    1.0842021724855044e-19}; // the first 64 iteration of arctan(2^-n) with n
                             // from 0 to 64
const double K = 1.646760258121;              // scale factor
const double Kopp = 0.6072529350088812561694; // 1/k
const double pi_half = 1.5707963267948966;
const double pi = 3.141592653589793;
const double pi_32 = 4.71238898038469;
const double pi_2 = 6.283185307179586;
const double zero = 0.0f;
const double one = 1.0f;
const double minus_one = -1.0f;

/** used to couple fixedpoint to corresponding value
 * @param T lenght of array
 * @param U type of llvm to couple
 */
template <typename U, int T = 0> struct pair_ftp_value {
  SmallVector<flttofix::FixedPointType, T> fpt;
  SmallVector<U, T> value;
  pair_ftp_value() {}
  pair_ftp_value(const SmallVector<flttofix::FixedPointType, T>  &fpt) : fpt(fpt), value() {}
  ~pair_ftp_value() {}
};
/** partial specialization for T=0
 * @param T lenght of array
 * @param U type of llvm to couple
 */
template <typename U> struct pair_ftp_value<U, 0> {
  flttofix::FixedPointType fpt;
  U value;
  pair_ftp_value() {}
  pair_ftp_value(const flttofix::FixedPointType &fpt) : fpt(fpt), value() {}
  ~pair_ftp_value() {}
};

inline void createFixedPointFromConst(
    llvm::LLVMContext &cont, FloatToFixed *ref, const float &current_float,
    const FixedPointType &match, llvm::Constant *&outI, FixedPointType &outF

);

} // namespace TaffoMathConstant
