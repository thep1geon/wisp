;; (set 'print_list (fn (l)
;;                      (set 'x (car l))
;;                      (if (= x nil)
;;                        nil
;;                        ((fn () 
;;                            (println x) 
;;                            (print_list (cdr l)))))))
;;
;; (print_list '(1 2 3))

(set 'x 32)
(set 'y 32)

(println (= x y))

(println +)
(println print)
(println print)
(println print)
