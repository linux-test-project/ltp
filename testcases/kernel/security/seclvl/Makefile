all: ioperm iopl create_suid_file create_guid_file create_suid_node \
     create_guid_node

ioperm: ioperm.c
	gcc -o ioperm ioperm.c

iopl: iopl.c
	gcc -o iopl iopl.c

create_suid_file: create_suid_file.c
	gcc -o create_suid_file create_suid_file.c

create_guid_file: create_guid_file.c
	gcc -o create_guid_file create_guid_file.c

create_suid_node: create_suid_node.c
	gcc -o create_suid_node create_suid_node.c

create_guid_node: create_guid_node.c
	gcc -o create_guid_node create_guid_node.c

clean:
	rm -f ioperm
	rm -f iopl
	rm -f create_suid_file
	rm -f create_suid_node
	rm -f create_guid_file
	rm -f create_guid_node
