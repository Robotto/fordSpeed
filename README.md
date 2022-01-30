
fordSpeed - a nixie tube speedometer for a Ford 
Model A. Interprets bytes from serial as raw unsigned 
integer values. Creates the proper binary pattern and 
shifts it out to a HV5622 32bit high voltage shift 
register, which drives three Nixie Tubes This works because 
the nixies are connected in a way that allows us to shift a 
bit n-times per digit (with value n), to light up the 
corresponding value of that digit. Each tube uses 10 bits, 
the HV5622 has 32.. so we've wasted two bits... sorry. 
Copyleft 2022 ~Robotto

