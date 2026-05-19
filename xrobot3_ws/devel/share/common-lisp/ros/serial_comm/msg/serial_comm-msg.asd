
(cl:in-package :asdf)

(defsystem "serial_comm-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "RobotControl" :depends-on ("_package_RobotControl"))
    (:file "_package_RobotControl" :depends-on ("_package"))
  ))