if ("${OC_VERSION}" STREQUAL "")
	set(OC_VERSION "(unknown-revision)")
	find_package(Git QUIET)

	if (GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} status --porcelain
			RESULT_VARIABLE STATUS_SUCCESS
			OUTPUT_VARIABLE STATUS_OUT
			ERROR_VARIABLE STATUS_ERR
		)

		if(${STATUS_SUCCESS} EQUAL 0)
			if("${STATUS_OUT}" STREQUAL "")
				set(DIRTY_STR "")
			else()
				set(DIRTY_STR "-dirty")
			endif()

			execute_process(
				COMMAND ${GIT_EXECUTABLE} show -s "--format=%h${DIRTY_STR} (%s, %cs)" HEAD
				RESULT_VARIABLE STATUS_SUCCESS
				OUTPUT_VARIABLE STATUS_OUT
				ERROR_VARIABLE STATUS_ERR
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			if (${STATUS_SUCCESS} EQUAL 0)
				set(OC_VERSION "${STATUS_OUT}")
			endif()
		endif()
	endif()
endif()

configure_file(${IN} ${OUT})
