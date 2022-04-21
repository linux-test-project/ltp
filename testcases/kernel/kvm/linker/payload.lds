TARGET(binary)

SECTIONS
{
	.data :
	{
		kvm_payload_start = .;
		KEEP(*(.data))
		kvm_payload_end = .;
	}
}
