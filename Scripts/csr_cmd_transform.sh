	 #!/bin/bash
	 
	 sed -i 's/csr_access *cpu read/csr_access --cpu-read/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *cpu write/csr_access --cpu-write/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *cpu rawread/csr_access --cpu-raw-read/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *cpu_cpld read/csr_access --cpu-cpld-read/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *cpu_cpld write/csr_access --cpu-cpld-write/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *nc read/csr_access --nc-read/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *nc write/csr_access --nc-write/g' $1/*.sh $1/*.txt 
	 sed -i 's/csr_access *nc_cpld read/csr_access --nc-cpld-read/g' $1/*.sh $1/*.txt 
 	 sed -i 's/csr_access *nc_cpld write/csr_access --nc-cpld-write/g' $1/*.sh $1/*.txt
 	 sed -i 's/csr_access *io_cpld read/csr_access --io-cpld-read/g' $1/*.sh $1/*.txt 
 	 sed -i 's/csr_access *io_cpld write/csr_access --io-cpld-write/g' $1/*.sh $1/*.txt 
	 sed -i 's/csr_access *mc/csr_access --mc/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *mc_pod read/csr_access --mc-pod-read/g' $1/*.sh $1/*.txt 
	 sed -i 's/csr_access *mc_pod write/csr_access --mc-pod-write/g' $1/*.sh $1/*.txt 
	 sed -i 's/csr_access *memled turnon/csr_access --memled-turnon/g' $1/*.sh $1/*.txt
	 sed -i 's/csr_access *memled turnoff/csr_access --memled-turnoff/g' $1/*.sh $1/*.txt 
	 echo DONE !!
