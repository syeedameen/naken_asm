;; VU1 has 16k of program program memory and 16k data memory.
;; The structure sent from the core unit to VU1 is:
;;
;; struct _object
;; {
;;   float sin_rx, cos_rx, sin_ry, cos_ry;
;;   float sin_rz, cos_rz, unused_0, unused_1;
;;   float 0, dz, dy, dx;
;;   int count, should_rotate, unused_2, unused_3;
;;   uint64_t GIF_PACKET_PRIMITIVE[2]
;;   uint64_t prim[2]
;;   uint64_t GIF_PACKET_TRIANGLES[2]
;;   struct _data
;;   {
;;     struct _color
;;     {
;;       uint8_t r;
;;       uint8_t g;
;;       uint8_t b;
;;       uint8_t a;
;;       uint32_t q = 0x3f80_0000;
;;       uint64_t reg_rgbaq = REG_RGBAQ;
;;     };
;;     float x, y, x, 0;
;;   } data[count];
;; }
;;
;; This structure will start at location 0 in data.
;;
;; In a register loaded from memory is:
;;                          { w,   z, y, x }
;;                   vf00 = { 1.0, 0, 0, 0 }
;;                   vi00 = 0

.ps2_ee_vu1
.include "playstation2/registers_gs_gp.inc"

.org 0
start:
  ;; vi01 points to the source GIF packet.
  ;; vi02 points to the rotation, position information.
  ;; vi03 holds the count.
  ;; vi04 points to the next point to be processed.
  ;; vi05 used to calculate pi / 2.
  ;; vi10 do_rotation_x
  ;; vi11 do_rotation_y
  ;; vi12 do_rotation_z
  ;; vf02 position change { 0, dz, dy, dx }
  ;; vf05 sin(rx), cos(rx), sin(ry), cos(ry)
  ;; vf06 sin(rz), cos(rz), 0.0, 0.0
  ;; vf10 is the next point being processed>
  ;; vf11 row 0 of transformation matrix X
  ;; vf12 row 1 of transformation matrix X
  ;; vf13 row 2 of transformation matrix X
  ;; vf14 row 0 of transformation matrix Y
  ;; vf15 row 1 of transformation matrix Y
  ;; vf16 row 2 of transformation matrix Y
  ;; vf17 row 0 of transformation matrix Z
  ;; vf18 row 1 of transformation matrix Z
  ;; vf19 row 2 of transformation matrix Z
  ;; vf30 temp for rotations
  ;; vf31 temp for rotations
  nop                         iaddiu vi01, vi00, 4
  nop                         iaddiu vi04, vi00, 8

  ;; Setup count/vi03, vi10=do_rot_x, vi11=do_rot_y, vi12=do_rot_z
  nop                         ilw.x vi03, 3(vi00)
  nop                         ilw.y vi10, 3(vi00)
  nop                         ilw.z vi11, 3(vi00)
  nop                         ilw.w vi12, 3(vi00)

  ;; Load sin / cos from the data sent from EE
  nop                         lq.xyzw vf05, 0(vi00)
  nop                         lq.xyzw vf06, 1(vi00)

  ;; vf02 = { 0, dz, dy, dx }
  nop                         lq.xyzw vf02, 2(vi00)

  ;; Build rotation matrixes if needed

  ;; Check if rotation X matrix should be made
  nop                         ibeq vi10, vi00, skip_rot_matrix_x
  nop                         nop

  ;; Build X rotational matrix
  nop                         move.xyzw vf11, vf00
  nop                         move.xyzw vf12, vf00
  nop                         move.xyzw vf13, vf00
  addw.x vf11, vf00, vf00w    nop
  addy.y vf12, vf00, vf05y    nop
  subx.z vf12, vf00, vf05x    nop
  addy.z vf13, vf00, vf05y    nop
  addy.y vf13, vf00, vf05y    nop

skip_rot_matrix_x:

  ;; Check if rotation Y matrix should be made
  nop                         ibeq vi11, vi00, skip_rot_matrix_y
  nop                         nop

  ;; Build Y rotational matrix
  nop                         move.xyzw vf14, vf00
  nop                         move.xyzw vf15, vf00
  nop                         move.xyzw vf16, vf00
  addw.x vf14, vf00, vf05w    nop
  addz.z vf14, vf00, vf05z    nop
  addw.y vf15, vf00, vf00w    nop
  subz.x vf16, vf00, vf05z    nop
  addw.z vf16, vf00, vf05w    nop

skip_rot_matrix_y:

  ;; Check if rotation Z matrix should be made
  nop                         ibeq vi12, vi00, skip_rot_matrix_z
  nop                         nop

  ;; Build Z rotational matrix
  nop                         move.xyzw vf17, vf00
  nop                         move.xyzw vf18, vf00
  nop                         move.xyzw vf19, vf00
  addy.x vf17, vf00, vf06y    nop
  subx.y vf17, vf00, vf06x    nop
  addx.x vf18, vf00, vf06x    nop
  addy.y vf18, vf00, vf06y    nop
  addw.z vf19, vf00, vf00w    nop
skip_rot_matrix_z:

next_point:
  ;; Load next point from data RAM.
  nop                         lq.xyzw vf10, 0(vi04)

  ;; Check if rotation X should be done
  nop                         ibeq vi10, vi00, skip_rot_x
  nop                         nop

  ;; Multiply X rotation matrix with points
  mul.xyzw vf30, vf11, vf10   move vf31, vf00
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf12, vf10   mfp.x vf31, P
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf13, vf10   mfp.y vf31, P
  nop                         esum P, vf30
  nop                         waitp
  nop                         mfp.z vf31, P
  nop                         move vf10, vf31
skip_rot_x:

  ;; Check if rotation Y should be done
  nop                         ibeq vi11, vi00, skip_rot_y
  nop                         nop

  ;; Multiply Y rotation matrix with points
  mul.xyzw vf30, vf14, vf10   move vf31, vf00
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf15, vf10   mfp.x vf31, P
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf16, vf10   mfp.y vf31, P
  nop                         esum P, vf30
  nop                         waitp
  nop                         mfp.z vf31, P
  nop                         move vf10, vf31
skip_rot_y:

  ;; Check if rotation Z should be done
  nop                         ibeq vi12, vi00, skip_rot_z
  nop                         nop

  ;; Multiply Z rotation matrix with points
  mul.xyzw vf30, vf17, vf10   move vf31, vf00
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf18, vf10   mfp.x vf31, P
  nop                         esum P, vf30
  nop                         waitp
  mul.xyzw vf30, vf19, vf10   mfp.y vf31, P
  nop                         esum P, vf30
  nop                         waitp
  nop                         mfp.z vf31, P
  nop                         move.xyz vf10, vf31
skip_rot_z:

  ;; Transpose point by { 0, dz, dy, dx } vector.
  add.xyz vf10, vf10, vf02    nop

  ;; Convert to X and Y to fixed point 12:4 and Z to just an integer.
  ftoi4.xy vf10, vf10         nop
  ftoi0.z  vf10, vf10         nop

  ;; Save back into GIF packet.
  nop                         sq.xyzw vf10, 0(vi04)

  ;; Increment registers and finish for loop.
  nop                         isubiu vi03, vi03, 1
  nop                         iaddiu vi04, vi04, 2

  nop                         ibne vi03, vi00, next_point
  nop                         nop

  ;; Send GIF packet to the GS
  nop                         xgkick vi01
  nop                         nop

  ;; End micro program
  nop[E]                      nop
  nop                         nop

