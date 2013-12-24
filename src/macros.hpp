#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__,                        \
    62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,  \
    38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,  \
    14,13,12,11,10,9,8,7,6,5,4,3,2,1)

#define VA_NUM_ARGS_IMPL(                                                     \
    _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,   \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,  \
    _39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,  \
    _57,_58,_59,_60,_61,_62,N,...) N

#define MACRO_DISPATCHER(func, ...)                                           \
  MACRO_DISPATCHER_(func, VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

#define MACRO_DISPATCHER_(func, nargs, ...)                                   \
  MACRO_DISPATCHER__(func, nargs, __VA_ARGS__)

#define MACRO_DISPATCHER__(func, nargs, ...)                                  \
  func ## nargs ( __VA_ARGS__ )

#define MACRO_DISPATCHER_1ARG(func, arg, ...)                                 \
  MACRO_DISPATCHER_1ARG_(func, arg, VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

#define MACRO_DISPATCHER_1ARG_(func, arg, nargs, ...)                         \
  MACRO_DISPATCHER_1ARG__(func, arg, nargs, __VA_ARGS__)

#define MACRO_DISPATCHER_1ARG__(func, arg, nargs, ...)                        \
  func ## nargs ( arg, __VA_ARGS__ )

#define ARGN_PASTER(NARGS, ...) ARGN_PASTER##NARGS##(__VA_ARGS__)
#define TYPE_PASTER(NARGS, ...) TYPE_PASTER##NARGS##(__VA_ARGS__)
#define TYPE_ARG_CC(NARGS, ...) TYPE_ARG_CC##NARGS##(__VA_ARGS__)

#define ARGN_PASTER2( x, y, ...) y
#define ARGN_PASTER4( x, y, ...) y, ARGN_PASTER2( __VA_ARGS__)
#define ARGN_PASTER6( x, y, ...) y, ARGN_PASTER4( __VA_ARGS__)
#define ARGN_PASTER8( x, y, ...) y, ARGN_PASTER6( __VA_ARGS__)
#define ARGN_PASTER10(x, y, ...) y, ARGN_PASTER8( __VA_ARGS__)
#define ARGN_PASTER12(x, y, ...) y, ARGN_PASTER10(__VA_ARGS__)
#define ARGN_PASTER14(x, y, ...) y, ARGN_PASTER12(__VA_ARGS__)
#define ARGN_PASTER16(x, y, ...) y, ARGN_PASTER14(__VA_ARGS__)
#define ARGN_PASTER18(x, y, ...) y, ARGN_PASTER16(__VA_ARGS__)
#define ARGN_PASTER20(x, y, ...) y, ARGN_PASTER18(__VA_ARGS__)
#define ARGN_PASTER22(x, y, ...) y, ARGN_PASTER20(__VA_ARGS__)
#define ARGN_PASTER24(x, y, ...) y, ARGN_PASTER22(__VA_ARGS__)
#define ARGN_PASTER26(x, y, ...) y, ARGN_PASTER24(__VA_ARGS__)
#define ARGN_PASTER28(x, y, ...) y, ARGN_PASTER26(__VA_ARGS__)
#define ARGN_PASTER30(x, y, ...) y, ARGN_PASTER28(__VA_ARGS__)
#define ARGN_PASTER32(x, y, ...) y, ARGN_PASTER30(__VA_ARGS__)
#define ARGN_PASTER34(x, y, ...) y, ARGN_PASTER32(__VA_ARGS__)
#define ARGN_PASTER36(x, y, ...) y, ARGN_PASTER34(__VA_ARGS__)
#define ARGN_PASTER38(x, y, ...) y, ARGN_PASTER36(__VA_ARGS__)
#define ARGN_PASTER40(x, y, ...) y, ARGN_PASTER38(__VA_ARGS__)
#define ARGN_PASTER42(x, y, ...) y, ARGN_PASTER40(__VA_ARGS__)
#define ARGN_PASTER44(x, y, ...) y, ARGN_PASTER42(__VA_ARGS__)
#define ARGN_PASTER46(x, y, ...) y, ARGN_PASTER44(__VA_ARGS__)
#define ARGN_PASTER48(x, y, ...) y, ARGN_PASTER46(__VA_ARGS__)
#define ARGN_PASTER50(x, y, ...) y, ARGN_PASTER48(__VA_ARGS__)
#define ARGN_PASTER52(x, y, ...) y, ARGN_PASTER50(__VA_ARGS__)
#define ARGN_PASTER54(x, y, ...) y, ARGN_PASTER52(__VA_ARGS__)
#define ARGN_PASTER56(x, y, ...) y, ARGN_PASTER54(__VA_ARGS__)
#define ARGN_PASTER58(x, y, ...) y, ARGN_PASTER56(__VA_ARGS__)
#define ARGN_PASTER60(x, y, ...) y, ARGN_PASTER58(__VA_ARGS__)
#define ARGN_PASTER62(x, y, ...) y, ARGN_PASTER60(__VA_ARGS__)

#define TYPE_PASTER2( x, y, ...) x
#define TYPE_PASTER4( x, y, ...) x, TYPE_PASTER2( __VA_ARGS__)
#define TYPE_PASTER6( x, y, ...) x, TYPE_PASTER4( __VA_ARGS__)
#define TYPE_PASTER8( x, y, ...) x, TYPE_PASTER6( __VA_ARGS__)
#define TYPE_PASTER10(x, y, ...) x, TYPE_PASTER8( __VA_ARGS__)
#define TYPE_PASTER12(x, y, ...) x, TYPE_PASTER10(__VA_ARGS__)
#define TYPE_PASTER14(x, y, ...) x, TYPE_PASTER12(__VA_ARGS__)
#define TYPE_PASTER16(x, y, ...) x, TYPE_PASTER14(__VA_ARGS__)
#define TYPE_PASTER18(x, y, ...) x, TYPE_PASTER16(__VA_ARGS__)
#define TYPE_PASTER20(x, y, ...) x, TYPE_PASTER18(__VA_ARGS__)
#define TYPE_PASTER22(x, y, ...) x, TYPE_PASTER20(__VA_ARGS__)
#define TYPE_PASTER24(x, y, ...) x, TYPE_PASTER22(__VA_ARGS__)
#define TYPE_PASTER26(x, y, ...) x, TYPE_PASTER24(__VA_ARGS__)
#define TYPE_PASTER28(x, y, ...) x, TYPE_PASTER26(__VA_ARGS__)
#define TYPE_PASTER30(x, y, ...) x, TYPE_PASTER28(__VA_ARGS__)
#define TYPE_PASTER32(x, y, ...) x, TYPE_PASTER30(__VA_ARGS__)
#define TYPE_PASTER34(x, y, ...) x, TYPE_PASTER32(__VA_ARGS__)
#define TYPE_PASTER36(x, y, ...) x, TYPE_PASTER34(__VA_ARGS__)
#define TYPE_PASTER38(x, y, ...) x, TYPE_PASTER36(__VA_ARGS__)
#define TYPE_PASTER40(x, y, ...) x, TYPE_PASTER38(__VA_ARGS__)
#define TYPE_PASTER42(x, y, ...) x, TYPE_PASTER40(__VA_ARGS__)
#define TYPE_PASTER44(x, y, ...) x, TYPE_PASTER42(__VA_ARGS__)
#define TYPE_PASTER46(x, y, ...) x, TYPE_PASTER44(__VA_ARGS__)
#define TYPE_PASTER48(x, y, ...) x, TYPE_PASTER46(__VA_ARGS__)
#define TYPE_PASTER50(x, y, ...) x, TYPE_PASTER48(__VA_ARGS__)
#define TYPE_PASTER52(x, y, ...) x, TYPE_PASTER50(__VA_ARGS__)
#define TYPE_PASTER54(x, y, ...) x, TYPE_PASTER52(__VA_ARGS__)
#define TYPE_PASTER56(x, y, ...) x, TYPE_PASTER54(__VA_ARGS__)
#define TYPE_PASTER58(x, y, ...) x, TYPE_PASTER56(__VA_ARGS__)
#define TYPE_PASTER60(x, y, ...) x, TYPE_PASTER58(__VA_ARGS__)
#define TYPE_PASTER62(x, y, ...) x, TYPE_PASTER60(__VA_ARGS__)

#define TYPE_ARG_CC2( x, y, ...) x y
#define TYPE_ARG_CC4( x, y, ...) x y, TYPE_ARG_CC2( __VA_ARGS__)
#define TYPE_ARG_CC6( x, y, ...) x y, TYPE_ARG_CC4( __VA_ARGS__)
#define TYPE_ARG_CC8( x, y, ...) x y, TYPE_ARG_CC6( __VA_ARGS__)
#define TYPE_ARG_CC10(x, y, ...) x y, TYPE_ARG_CC8( __VA_ARGS__)
#define TYPE_ARG_CC12(x, y, ...) x y, TYPE_ARG_CC10(__VA_ARGS__)
#define TYPE_ARG_CC14(x, y, ...) x y, TYPE_ARG_CC12(__VA_ARGS__)
#define TYPE_ARG_CC16(x, y, ...) x y, TYPE_ARG_CC14(__VA_ARGS__)
#define TYPE_ARG_CC18(x, y, ...) x y, TYPE_ARG_CC16(__VA_ARGS__)
#define TYPE_ARG_CC20(x, y, ...) x y, TYPE_ARG_CC18(__VA_ARGS__)
#define TYPE_ARG_CC22(x, y, ...) x y, TYPE_ARG_CC20(__VA_ARGS__)
#define TYPE_ARG_CC24(x, y, ...) x y, TYPE_ARG_CC22(__VA_ARGS__)
#define TYPE_ARG_CC26(x, y, ...) x y, TYPE_ARG_CC24(__VA_ARGS__)
#define TYPE_ARG_CC28(x, y, ...) x y, TYPE_ARG_CC26(__VA_ARGS__)
#define TYPE_ARG_CC30(x, y, ...) x y, TYPE_ARG_CC28(__VA_ARGS__)
#define TYPE_ARG_CC32(x, y, ...) x y, TYPE_ARG_CC30(__VA_ARGS__)
#define TYPE_ARG_CC34(x, y, ...) x y, TYPE_ARG_CC32(__VA_ARGS__)
#define TYPE_ARG_CC36(x, y, ...) x y, TYPE_ARG_CC34(__VA_ARGS__)
#define TYPE_ARG_CC38(x, y, ...) x y, TYPE_ARG_CC36(__VA_ARGS__)
#define TYPE_ARG_CC40(x, y, ...) x y, TYPE_ARG_CC38(__VA_ARGS__)
#define TYPE_ARG_CC42(x, y, ...) x y, TYPE_ARG_CC40(__VA_ARGS__)
#define TYPE_ARG_CC44(x, y, ...) x y, TYPE_ARG_CC42(__VA_ARGS__)
#define TYPE_ARG_CC46(x, y, ...) x y, TYPE_ARG_CC44(__VA_ARGS__)
#define TYPE_ARG_CC48(x, y, ...) x y, TYPE_ARG_CC46(__VA_ARGS__)
#define TYPE_ARG_CC50(x, y, ...) x y, TYPE_ARG_CC48(__VA_ARGS__)
#define TYPE_ARG_CC52(x, y, ...) x y, TYPE_ARG_CC50(__VA_ARGS__)
#define TYPE_ARG_CC54(x, y, ...) x y, TYPE_ARG_CC52(__VA_ARGS__)
#define TYPE_ARG_CC56(x, y, ...) x y, TYPE_ARG_CC54(__VA_ARGS__)
#define TYPE_ARG_CC58(x, y, ...) x y, TYPE_ARG_CC56(__VA_ARGS__)
#define TYPE_ARG_CC60(x, y, ...) x y, TYPE_ARG_CC58(__VA_ARGS__)
#define TYPE_ARG_CC62(x, y, ...) x y, TYPE_ARG_CC60(__VA_ARGS__)
