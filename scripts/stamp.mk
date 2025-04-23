define declare_stamp
$(1).stamp_$(2): FORCE
	@echo "$3" | md5sum | cut -c1-32 > $(1).stamp_$(2).new; \
	if [ ! -f $(1).stamp_$(2) ] || ! cmp -s $(1).stamp_$(2) $(1).stamp_$(2).new; then \
		mv $(1).stamp_$(2).new $(1).stamp_$(2); \
	else \
		rm $(1).stamp_$(2).new; \
	fi
endef