;; some useful TL2 macros for emacs
;; you might want to copy this to your .emacs

;; create a metaclass
;; cursor must be before the keyword "class"
(fset 'wizard
      [(meta right) (meta right) (meta left) (control meta space)
       (meta w) (control x) (control f) (control y) C l a s s \. t c
       return c l a s s space (control y) C l a s s return s u p e r
       space C o n c r e t e C l a s s \( (control y) \) return m e t
       a space M e t a C l a s s return { return return n e w space :
       (control y) return { return space space i n s t a n c e space :
       : = space _ n e w return i n s t a n c e \. _ i n i t return i
       n s t a n c e return backspace backspace } return return } return
       (control x) (control s)])

;; cut a class out of a scratch buffer into its own file.
;; cursor must be at the word "class"
(fset 'cut-class
   [(meta right) right (control meta space) (meta w) (control x) (c
ontrol f) (control y) \. t c return (control x) b return (control a
) (control space) (control s) \; right (control x) (control x) (met
a w) (control x) b return (control y) (control x) b return (control
 x) (control x) (control s) c l a s s (control a)])

;;  ^.<slotname>$
;;  ^<foo>$
;; -->
;;  ^  instance.<slotname> := <slotname>$
;;  ^.<foo>$
(fset 'tl2-slot-assign
   [space space i n s t a n c e \. (control k) (control y) space :
= space (control y) right])
(add-hook 'tl2-mode-hook
   (function (lambda () (define-key tl2-mode-map "\C-c\C-v" 'tl2-slot-assign))))
