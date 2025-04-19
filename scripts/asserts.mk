# usage: $(call assert-gt, X, Y, msg)
define assert-gt
  $(if $(filter true,$(shell [ $($1) -gt $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be > $2 ($($2)) $(strip $3)))
endef

define assert-ge
  $(if $(filter true,$(shell [ $($1) -ge $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be >= $2 ($($2)) $(strip $3)))
endef

define assert-lt
  $(if $(filter true,$(shell [ $($1) -lt $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be < $2 ($($2)) $(strip $3)))
endef

define assert-le
  $(if $(filter true,$(shell [ $($1) -le $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be <= $2 ($($2)) $(strip $3)))
endef

define assert-eq
  $(if $(filter true,$(shell [ $($1) -eq $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be == $2 ($($2)) $(strip $3)))
endef

define assert-ne
  $(if $(filter true,$(shell [ $($1) -ne $($2) ] && echo true)),,$(error ASSERT FAIL: $1 ($($1)) must be != $2 ($($2)) $(strip $3)))
endef
