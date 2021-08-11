(import "lib/math/matrix.inc")

(enums +tri 0
	(enum i_v0 i_v1 i_v2 i_n))

(enums +mesh 0
	(enum verts norms tris))

(defun tri (i_v0 i_v1 i_v2 i_n)
	(list i_v0 i_v1 i_v2 i_n))

(defun mesh (verts norms tris)
	(list verts norms tris))

(defun gen-sphere (radius eps)
	; (gen-sphere radius eps) -> mesh
	(defq verts (list) norms (list) tris (list)
		y 1 eps2 (* eps 2) eps2_r (i2r eps2))
	;gen verts
	(while (< y eps)
		(defq x 0 ya (/ (* (i2r y) +real_2pi) eps2_r)
			yv (* radius (cos ya)) r (* radius (sin ya)))
		(while (< x eps2)
			(defq xa (/ (* (i2r x) +real_2pi) eps2_r)
				xv (* r (sin xa)) zv (* r (cos xa)))
			(push verts (vec3-r xv yv zv))
			(setq x (inc x)))
		(setq y (inc y)))
	;gen norms and tris for mid section strips
	(defq y 1)
	(while (< y (dec eps))
		(defq x 0)
		(while (< x eps2)
			(defq i0 (+ x (* (dec y) eps2))
				i1 (+ (% (inc x) eps2) (* (dec y) eps2))
				i2 (+ i0 eps2) i3 (+ i1 eps2)
				v0 (elem i0 verts) v1 (elem i1 verts)
				v2 (elem i2 verts) v3 (elem i3 verts))
			(push tris (tri i0 i2 i1 (length norms)))
			(push norms (vec-norm (apply reals (vec-cross-3d (vec-sub v0 v1) (vec-sub v0 v2)))))
			(push tris (tri i1 i2 i3 (length norms)))
			(push norms (vec-norm (apply reals (vec-cross-3d (vec-sub v2 v1) (vec-sub v2 v3)))))
			(setq x (inc x)))
		(setq y (inc y)))
	;gen verts, norms and tris for end cap fans
	(push verts (vec3-r +real_0 radius +real_0) (vec3-r +real_0 (neg radius) +real_0))
	(defq x 0 y (* eps2 (- eps 2)))
	(while (< x eps2)
		(defq i0 (+ x y) i1 (+ (% (inc x) eps2) y) i2 (dec (length verts))
			v0 (elem i0 verts) v1 (elem i1 verts) v2 (elem i2 verts))
		(push tris (tri i0 i2 i1 (length norms)))
		(push norms (vec-norm (apply reals (vec-cross-3d (vec-sub v0 v1) (vec-sub v0 v2)))))
		(defq i0 x i1 (% (inc x) eps2) i2 (- (length verts) 2)
			v0 (elem i0 verts) v1 (elem i1 verts) v2 (elem i2 verts))
		(push tris (tri i0 i1 i2 (length norms)))
		(push norms (vec-norm (apply reals (vec-cross-3d (vec-sub v2 v1) (vec-sub v2 v0)))))
		(setq x (inc x)))
	(each (# (push %0 +real_1)) verts)
	(mesh verts norms tris))