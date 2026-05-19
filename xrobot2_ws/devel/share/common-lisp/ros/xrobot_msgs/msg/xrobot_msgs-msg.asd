
(cl:in-package :asdf)

(defsystem "xrobot_msgs-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "ParamConfig" :depends-on ("_package_ParamConfig"))
    (:file "_package_ParamConfig" :depends-on ("_package"))
    (:file "SensorState" :depends-on ("_package_SensorState"))
    (:file "_package_SensorState" :depends-on ("_package"))
    (:file "VersionInfo" :depends-on ("_package_VersionInfo"))
    (:file "_package_VersionInfo" :depends-on ("_package"))
  ))