APPS= apsp bc sssp community connected_components dfs bfs pagerank triangle_counting tsp

export CRONO_DIR=$(PWD)

SUBDIRS=$(addprefix $(CRONO_DIR)/apps/,$(APPS))
.PHONY: all clean
.PHONY: $(SUBDIRS)
.PHONY: libs

all: libs $(SUBDIRS)
	for dir in $^; do \
		$(MAKE) $(MAKECMDGOALS) -C $$dir; \
	done

clean: libs $(SUBDIRS)
	for dir in $^; do \
		$(MAKE) $(MAKECMDGOALS) -C $$dir; \
	done
