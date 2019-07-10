(use-modules (srfi srfi-9) (srfi srfi-1))
; Props:
;   Bitmask (28 bits)
;   Difficulty (0-2)
;   For 6 characters:
;     For 5 weapons:
;       weapon level (2 bits)
;     hp level (2 bits)

; Define the password character set
(define charset "BCDFGHJKLMNPQRSTVWXYZ0123456789@")

(define-record-type  chara (make-chara hplevel weapons) chara?
                     (hplevel     chara-hplevel)
                     (weapons     chara-weapons))
(define-record-type  data (make-data bits difficulty charas) data?
                     (bits        data-bits)
                     (difficulty  data-difficulty)
                     (charas      data-charas))

; Some syntax helpers.
; Syntactic shortcut: (map f (iota a b c))
;  can be written as: (maptimes f a b c)
(define (maptimes f . xs)  (map f (apply iota xs)))

; Recursive calls:    (lambda (f a) (f f a)) (lambda (i x) (i (+ x 1))) 0
;  can be written as: recursively (lambda (i x) (i (+ x 1))) 0
(define (recursively . xs) (apply (car xs) xs))

; Currying shortcut:  (lambda (c d) (somefunc a b c d))
;  can be written as: (curry somefunc a b)
(define (curry f . xs)     (lambda ys (apply f (append xs ys))))

; Mathematical algorithms for the password encoding & decoding
(define (byte x)           (logand #xFF x))
(define (perm sum x)       (+ #x3D (logxor #x5A (ash (* (byte (+ sum x)) 257) -7))))
(define (sum3 sum x)       (logand 7 (+ sum (* 3 x))))
(define (mod9 sum)         (modulo (+ sum 9) 9))

(define (compose . xs)
  (if (null? xs) 0
    (+ (logand (car xs) (- (ash 1 (car (cdr xs))) 1))
       (ash (apply compose (cdr (cdr xs))) (car (cdr xs))))))

; pass-encode: Input is a data record. Output: password string.
(define (pass-encode in)
  ; First, pack the input data into bytes.
  (let* ((hp    (lambda (p)   (-           (chara-hplevel (list-ref (data-charas in) p)) 1)   ))
         (wpn   (lambda (p w) (- (list-ref (chara-weapons (list-ref (data-charas in) p)) w) 1)))
         (l36   (apply append (maptimes (lambda (p) (append (maptimes (curry wpn p) 5) (list (hp p)))) 6)))
         (bits  (compose  (data-bits in) 24  0 2  (data-difficulty in) 2  (ash (data-bits in) -24) 4))
         (bytes (append (maptimes (lambda (s) (apply + (map ash (maptimes (curry list-ref l36) 4 s) '(6 4 2 0)))) 9 0 4)
                        (maptimes (curry ash bits) 4 0 -8)))
         ; Calculate checksum
         (csum (apply - 0 bytes))
         (c    (byte (logxor csum #x5A)))
  ; In a single pass, encrypt the data and convert each byte into pairs of character & position.
  )(list->string (recursively (lambda (iter x sum w)
       (cons (string-ref charset (logand (if (null? w) c          (+ (car w) (ash sum -3))) 31))
       (cons (integer->char (+ 49        (if (null? w) (ash c -5) (mod9 (- (logand 7 (+ (ash (car w) -5) sum)) (sum3 csum x))))))
             (if (null? w) w (cons #\space (iter iter (+ x 1) (perm sum x) (cdr w)))))))
    0 csum bytes))))

; pass-decode: Input is a password string. Output is a pair of (error index, data record)
(define (pass-decode in)
  ; First, decode the string into pairs of character & position.
  (let* ((char    (lambda (n) (string-index charset      (string-ref in    (* n 3)   ))    ))
         (pos     (lambda (n) (- (string->number (string (string-ref in (+ (* n 3) 1)))) 1)))
         ; Decode the checksum byte.
         (csum    (logxor #x5A (compose (char 13) 5 (pos 13) 3)))
         ; Construct bytes from the characters and positions, decrypting the data in a single pass.
         (bytes (recursively (lambda (iter x sum  error?)
             (let ((y (mod9 (+ (pos x) (sum3 csum x)))))
                 (if (= x 13) (list error?)
                              (cons (compose (- (char x) (ash sum -3)) 5 (- y sum) 3)
                                    (iter iter (+ x 1) (perm sum x) (if (= 8 y) (+ x 1) error?))))))
                 0 csum 0))
         (y (curry list-ref bytes))
  ; Extract the bytes into individual values. (l n) = extracts 2-bit values +1
  )(cons (if (zero? (byte (apply + csum bytes))) (y 13) 14)             ; error code
         (let ((l (lambda (n) (+ 1 (logand 3 (ash (y (ash n -2)) (- 0 (logand 7 (* n 2)))))))))
              (make-data
                  (compose (y 9) 8 (y 10) 8 (y 11) 8 (ash (y 12) -4) 8) ; bits
                  (- (l 49) 1)                                          ; difficulty
                  (maptimes (lambda (n) (make-chara (l (+ n 5)) (maptimes l 5 n))) 6))))))


;(define input (make-data #xFFF0FDF 0 (list (make-chara 4 (list 4 4 4 4 4))
;                                           (make-chara 4 (list 4 4 4 4 4))
;                                           (make-chara 4 (list 4 4 4 4 4))
;                                           (make-chara 4 (list 4 4 4 4 4))
;                                           (make-chara 4 (list 4 4 4 4 4))
;                                           (make-chara 4 (list 4 4 4 4 4)))))
 (define input (make-data #x0000000 3 (list (make-chara 1 (list 1 1 1 1 1))
                                            (make-chara 1 (list 1 1 1 1 1))
                                            (make-chara 1 (list 1 1 1 1 1))
                                            (make-chara 1 (list 1 1 1 1 1))
                                            (make-chara 1 (list 1 1 1 1 1))
                                            (make-chara 1 (list 1 1 1 1 1)))))


(define (pretty-print str)
  (let* ((char-if (lambda (i k) (if (= k (char->integer (string-ref str (+ i 1)))) (string-ref str i) #\.)))
         (half    (lambda (start)
    (apply append (maptimes (lambda (n) (append
    (apply append (maptimes (lambda (i) (append (maptimes (curry char-if i) 3 n) '(#\space))
                  ) 7 start 3))                                                  '(#\newline))
                  ) 3 49 3)))))
    (list->string (append (half 0) (cons #\newline (half 21))))))

; Encoding a password:
(define str (pass-encode input))
(display str)
(display "\n")
(display (pretty-print str))
(display "-------------------\n")

; Decoding a password:
;(define str2 "@1 87 31 C9 W7 T9 Y1 P5 G2 L3 B7 T5 46 D6")
(define str2 "G9 J9 C5 Y9 L8 94 V1 Z5 Q2 84 P8 38 12 14")
(display (pass-decode str2)) (display "\n")
(display (pretty-print str2))
(display "-------------------\n")
