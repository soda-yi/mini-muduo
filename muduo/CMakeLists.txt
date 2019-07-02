aux_source_directory(. muduosrcs)
add_library(muduo STATIC ${muduosrcs})
install(TARGETS muduo DESTINATION lib)