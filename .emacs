(setq backup-directory-alist
      `(("." . "~/.emacs_backups")))

(require 'package)
(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/") t)
(package-initialize)

(setq custom-file "~/.emacs_custom.el")

(column-number-mode)
(tool-bar-mode 0)
(menu-bar-mode 0)
(scroll-bar-mode 0)
(ido-mode)
(ido-everywhere)
(setq ido-enable-flex-matching t)

(global-display-line-numbers-mode)
(setq display-line-numbers-type 'relative)
(setq inhibit-startup-screen t)
(setq-default tab-width 4)
(setq-default indent-tabs-mode nil)

;; (set-face-attribute 'default nil
;;                     :font "CaskaydiaMonoNerdFont"
;;                     :height 170
;;                     :weight 'light)

(set-face-attribute 'default nil
                    :font "Monaspace Neon NF"
                    :height 170
                    :weight 'light)

(use-package magit
    :ensure t)

(use-package vterm
    :ensure t)

(use-package gruber-darker-theme
    :ensure t
    :init
    (load-theme 'gruber-darker t))

(use-package smex
    :ensure t)
(global-set-key (kbd "M-x") 'smex)
(global-set-key (kbd "M-X") 'smex-major-mode-commands)

(use-package yaml-mode
    :ensure t
    :mode ("\\.clang-format\\'")
    )

(use-package cmake-mode
    :ensure t
    )

;; (add-to-list 'load-path "/home/liu/vendored/slang-mode")
;; (require 'slang-mode)

(use-package slang-mode
    :load-path "home/liu/vendored/slang-mode"
    :mode ("\\.slang\\'"))

(add-to-list 'auto-mode-alist '("\\.xprofile\\'" . sh-mode))
(add-to-list 'auto-mode-alist '("\\.cppm\\'" . c++-mode))

(use-package eglot
    :ensure t
    :hook
    ((c++-mode c-mode) . eglot-ensure)
    :config
    (add-to-list 'eglot-server-programs
                 '(c++-mode . ("clangd")))
    (add-to-list 'eglot-server-programs
	             '(c-mode . ("clangd")))
    )

(add-hook 'eglot-managed-mode-hook
          (lambda ()
              (flymake-mode 0)))

(setq c-basic-offset 4)
(setq cmake-tab-width 4)
(setq lisp-body-indent 4)
;; (setq slang-indent-offset 4)


(put 'set-goal-column 'disabled nil)
