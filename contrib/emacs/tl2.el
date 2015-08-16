;;; tl2.el --- TL2 process in a buffer. Adapted from cmuscheme.el

;; Author: Axel Wienberg <wienberg@dbis1.informatik.uni-hamburg.de>

;; I guess this file is GPLed

;;; Commentary:

;;;    This is a customisation of comint-mode (see comint.el)

(require 'tl2-mode)
(require 'comint)

;;; INFERIOR TL2 MODE STUFF
;;;============================================================================

(defvar inferior-tl2-mode-hook nil
  "*Hook for customising inferior-tl2 mode.")
(defvar inferior-tl2-mode-map nil)

(cond ((not inferior-tl2-mode-map)
       (setq inferior-tl2-mode-map
	     (copy-keymap comint-mode-map))
       (define-key inferior-tl2-mode-map "\M-\C-x" ;gnu convention
	           'tl2-send-definition)
       (define-key inferior-tl2-mode-map "\C-c\C-l" 'tl2-load-file)
       (tl2-mode-commands inferior-tl2-mode-map)))

;; Install the process communication commands in the tl2-mode keymap.
(define-key tl2-mode-map "\M-\C-x" 'tl2-send-definition);gnu convention
(define-key tl2-mode-map "\C-c\C-e" 'tl2-send-definition)
(define-key tl2-mode-map "\C-c\M-e" 'tl2-send-definition-and-go)
(define-key tl2-mode-map "\C-c\C-r" 'tl2-send-region)
(define-key tl2-mode-map "\C-c\M-r" 'tl2-send-region-and-go)
(define-key tl2-mode-map "\C-c\C-z" 'switch-to-tl2)
(define-key tl2-mode-map "\C-c\C-l" 'tl2-load-file)

(defvar tl2-buffer)

(defvar tl2-prompt "|>"
  "regexp matching the Tl2 prompt")
(defvar tl2-input-terminator ";"
  "regexp matching the terminator of a user command")

(defun inferior-tl2-mode ()
  "Major mode for interacting with an inferior Tl2 process.

The following commands are available:
\\{inferior-tl2-mode-map}

A Tl2 process can be fired up with M-x run-tl2.

Customisation: Entry to this mode runs the hooks on comint-mode-hook and
inferior-tl2-mode-hook (in that order).

You can send text to the inferior Tl2 process from other buffers containing
Tl2 source.  
    switch-to-tl2 switches the current buffer to the Tl2 process buffer.
    tl2-send-definition sends the current definition to the Tl2 process.
    tl2-send-region sends the current region to the Tl2 process.

    tl2-send-definition-and-go, tl2-compile-definition-and-go,
        and tl2-send-region-and-go
        switch to the Tl2 process buffer after sending their text.
For information on running multiple processes in multiple buffers, see
documentation for variable tl2-buffer.

Commands:
Return after the end of the process' output sends the text from the 
    end of process to point.
Delete converts tabs to spaces as it moves back.
Paragraphs are separated only by blank lines.
If you accidentally suspend your process, use \\[comint-continue-subjob]
to continue it."
  (interactive)
  (comint-mode)
  ;; Customise in inferior-tl2-mode-hook
  (setq comint-prompt-regexp (concat "^[^\n]*" tl2-prompt " *"))
  (tl2-mode-variables)
  (setq major-mode 'inferior-tl2-mode)
  (setq mode-name "Inferior Tl2")
  (setq mode-line-process '(":%s"))
  (use-local-map inferior-tl2-mode-map)
  (setq comint-input-filter (function tl2-input-filter))
  (setq comint-get-old-input (function tl2-get-old-input))
  (run-hooks 'inferior-tl2-mode-hook))

(defvar inferior-tl2-filter-regexp "\\`\\s *\\S ?\\S ?\\s *\\'"
  "*Input matching this regexp are not saved on the history list.
Defaults to a regexp ignoring all inputs of 0, 1, or 2 letters.")

(defun tl2-input-filter (str)
  "Don't save anything matching inferior-tl2-filter-regexp"
  (not (string-match inferior-tl2-filter-regexp str)))

(defun re-search-backward-end (re)
  "search backwards for regexp <re>. Leaves point at END of match, or
at beginning of buffer if not found."
  (if (re-search-backward re nil 'lala)
      (goto-char (match-end 0))))

(defvar tl2-command-separator-regexp
  (concat tl2-input-terminator "\\|" tl2-prompt)
  "regexp preceding the beginning of a tl2 command")

(defun tl2-get-old-input ()
  "Snarf the command ending at point"
  (save-excursion
    (skip-syntax-backward "-")
    (if (not (= (preceding-char) ?\;))
	(search-forward ";"))
    (let ((end (point)))
      (backward-char)  ;; across semicolon
      (re-search-backward-end tl2-command-separator-regexp)
      ;; now after prompt or previous semicolon
      (skip-syntax-forward "-")
      (buffer-substring (point) end))))


(defun tl2-args-to-list (string)
  (let ((where (string-match "[ \t]" string)))
    (cond ((null where) (list string))
	  ((not (= where 0))
	   (cons (substring string 0 where)
		 (tl2-args-to-list (substring string (+ 1 where)
						 (length string)))))
	  (t (let ((pos (string-match "[^ \t]" string)))
	       (if (null pos)
		   nil
		 (tl2-args-to-list (substring string pos
						 (length string)))))))))

(defvar tl2-program-name "tycoon -store TL2 -restart"
  "*Program invoked by the run-tl2 command")

;;;###autoload
(defun run-tl2 (cmd)
  "Run an inferior Tl2 process, input and output via buffer *tl2*.
If there is a process already running in *tl2*, just switch to that buffer.
With argument, allows you to edit the command line (default is value
of tl2-program-name).  Runs the hooks from inferior-tl2-mode-hook
\(after the comint-mode-hook is run).
\(Type \\[describe-mode] in the process buffer for a list of commands.)"

  (interactive (list (if current-prefix-arg
			 (read-string "Run Tl2: " tl2-program-name)
			 tl2-program-name)))
  (if (not (comint-check-proc "*tl2*"))
      (let ((cmdlist (tl2-args-to-list cmd)))
	(set-buffer (apply 'make-comint "tl2" (car cmdlist)
			   nil (cdr cmdlist)))
	(inferior-tl2-mode)))
  (setq tl2-program-name cmd)
  (setq tl2-buffer "*tl2*")
  (switch-to-buffer "*tl2*"))


(defun tl2-send-region (start end)
  "Send the current region to the inferior Tl2 process."
  (interactive "r")
  (comint-send-region (tl2-proc) start end)
  (comint-send-string (tl2-proc) "\n"))

(defun tl2-send-definition ()
  "Send the current definition to the inferior Tl2 process."
  (interactive)
  (save-excursion
   (if (not (= (preceding-char) ?\;))
       (search-forward ";"))
   (let ((end (point)))
     (backward-char)
     (if (search-backward ";" nil 'or-bob)
	 (forward-char))
     (skip-syntax-forward "-")
     (tl2-send-region (point) end))))

(defun switch-to-tl2 (eob-p)
  "Switch to the tl2 process buffer.
With argument, positions cursor at end of buffer."
  (interactive "P")
  (if (get-buffer tl2-buffer)
      (pop-to-buffer tl2-buffer)
      (error "No current process buffer. See variable tl2-buffer."))
  (cond (eob-p
	 (push-mark)
	 (goto-char (point-max)))))

(defun tl2-send-region-and-go (start end)
  "Send the current region to the inferior Tl2 process.
Then switch to the process buffer."
  (interactive "r")
  (tl2-send-region start end)
  (switch-to-tl2 t))

(defun tl2-send-definition-and-go ()
  "Send the current definition to the inferior Tl2. 
Then switch to the process buffer."
  (interactive)
  (tl2-send-definition)
  (switch-to-tl2 t))

(defvar tl2-source-modes '(tl2-mode)
  "*Used to determine if a buffer contains Tl2 source code.
If it's loaded into a buffer that is in one of these major modes, it's
considered a tl2 source file by tl2-load-file and tl2-compile-file.
Used by these commands to determine defaults.")

(defvar tl2-prev-l/c-dir/file nil
  "Caches the last (directory . file) pair.
Caches the last pair used in the last tl2-load-file or
tl2-compile-file command. Used for determining the default in the 
next one.")

(defun tl2-load-file (file-name)
  "Load a Tl2 file into the inferior Tl2 process."
  (interactive (comint-get-source "Load Tl2 file: " tl2-prev-l/c-dir/file
				  tl2-source-modes t)) ; T because LOAD 
                                                          ; needs an exact name
  (comint-check-source file-name) ; Check to see if buffer needs saved.
  (setq tl2-prev-l/c-dir/file (cons (file-name-directory    file-name)
				       (file-name-nondirectory file-name)))
  (comint-send-string (tl2-proc) (concat "DO load \""
					    file-name
					    "\";\n")))


(defvar tl2-buffer nil "*The current tl2 process buffer.

MULTIPLE PROCESS SUPPORT
===========================================================================
Inferior-Tl2.el supports, in a fairly simple fashion, running multiple Tl2
processes. To run multiple Tl2 processes, you start the first up with
\\[run-tl2]. It will be in a buffer named *tl2*. Rename this buffer
with \\[rename-buffer]. You may now start up a new process with another
\\[run-tl2]. It will be in a new buffer, named *tl2*. You can
switch between the different process buffers with \\[switch-to-buffer].

Commands that send text from source buffers to Tl2 processes --
like tl2-send-definition or tl2-compile-region -- have to choose a
process to send to, when you have more than one Tl2 process around. This
is determined by the global variable tl2-buffer. Suppose you
have three inferior Tl2s running:
    Buffer	Process
    foo		tl2
    bar		tl2<2>
    *tl2*    tl2<3>
If you do a \\[tl2-send-definition-and-go] command on some Tl2 source
code, what process do you send it to?

- If you're in a process buffer (foo, bar, or *tl2*), 
  you send it to that process.
- If you're in some other buffer (e.g., a source file), you
  send it to the process attached to buffer tl2-buffer.
This process selection is performed by function tl2-proc.

Whenever \\[run-tl2] fires up a new process, it resets tl2-buffer
to be the new process's buffer. If you only run one process, this will
do the right thing. If you run multiple processes, you can change
tl2-buffer to another process buffer with \\[set-variable].

More sophisticated approaches are, of course, possible. If you find yourself
needing to switch back and forth between multiple processes frequently,
you may wish to consider ilisp.el, a larger, more sophisticated package
for running inferior Lisp and Tl2 processes. The approach taken here is
for a minimal, simple implementation. Feel free to extend it.")

(defun tl2-proc ()
  "Returns the current tl2 process. See variable tl2-buffer."
  (let ((proc (get-buffer-process (if (eq major-mode 'inferior-tl2-mode)
				      (current-buffer)
				      tl2-buffer))))
    (or proc
	(error "No current process. See variable tl2-buffer"))))


;;; Do the user's customisation...

(defvar inferior-tl2-load-hook nil
  "This hook is run when inferior-tl2 is loaded in.
This is a good place to put keybindings.")
	
(run-hooks 'inferior-tl2-load-hook)


;;; CHANGE LOG
;;; ===========================================================================
;;; 4/96 Axel
;;;     created from cmuscheme.el.
;;;     renamed scheme->tl2 everywhere, kicked out compilation stuff.
;;;     there are no sexps, so we have to depend on prompt |> and semicolon ;


(provide 'tl2)

;;; tl2.el ends here
