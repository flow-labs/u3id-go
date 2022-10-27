# u3id golang bindings

The U3ID is broken up into 3 components, the integer time component, followed by the decimal time component, followed by the randomized chaotic component. The length of the integer time component will constrain the number of seconds from epoch that the ID can hold. With an integer time component length of 32 bits, the ID has room for the number of seconds since epoch up until the year 2106. The length of the decimal time component constrains the precision of time that the ID contains. With a decimal time component length of 10 bits, the precision is approximately 1ms. The decimal time component has a maximum allowed length of 30 bits which corresponds to approximately ns precision, which is the highest precision available on most machines. The random chaotic component will fill in the rest of the ID up until the total length of the ID. The chaotic component is filled with random bits by default.

The user has the option to provide any of the components themselves. In this case, the user provided integer time component can be any integer. The decimal time component should be in units of nanoseconds, which is important for rounding. The decimal time component will be rounded to the number of decimal places specified as a fraction of a full second. The user can also supply a text string that will be hashed using SHA512 into the chaotic component of the ID.

## Usage

Install the u3id Golang bindings with the following command:
```bash
go get github.com/flow-labs/u3idgo
```

Import the u3id functions into your golang script by adding the following line:
```golang
import (
	. "github.com/flow-labs/u3idgo"
)
```

This module contains several functions to generate u3ids in different modes that depend on the usecase. Here we will discribe the different modes and functions:

Note: All functions return two values, the first value is the u3id as a golang byte array, and the second value is an error. If the error != nil then something has gone wrong. Don't forget to check for this case in your code.

1. The first mode is called the standard mode. In this mode, the user provides the lengths in bits of the integer time component, the decimal time component, and the total length of the u3id.
    - The function for this mode has the following signature:
    ```golang
        func U3id_s(
	        timestamp_integer_part_length_bits int,
	        timestamp_decimal_part_length_bits int,
	        total_length_bits int,
        ) ([]byte, error) 
   ```
3. The second mode is called the chaotic mode. In this mode, the user provides the lengths in bits of the integer time component, the decimal time component, the total length of the u3id, and a text string to be hashed into the chaotic component.
    - The function for this mode has the following signature:
    ```golang
        func U3id_c(
	        timestamp_integer_part_length_bits int,
	        timestamp_decimal_part_length_bits int,
	        total_length_bits int,
	        chaotic_part_seed string,
        ) ([]byte, error)
   ```
4. The third mode is called the time mode. In this mode, the user provides the lengths in bits of the integer time component, the decimal time component, and the total length of the u3id. The user also provides the values of the integer and decimal time components. The decimal time component is in units of nanoseconds.
    - The function for this mode has the following signature:
    ```golang
        func U3id_t(
	        timestamp_integer_part_length_bits int,
	        timestamp_decimal_part_length_bits int,
	        total_length_bits int,
	        integer_time_part int64,
	        decimal_time_part_ns int32,
        ) ([]byte, error)
   ```
5. The fourth mode is called the all mode. In this mode the user provides all parameters.
   The function for this mode has the following signature:
    ```golang
        func U3id_a(
	        timestamp_integer_part_length_bits int,
	        timestamp_decimal_part_length_bits int,
	        total_length_bits int,
	        integer_time_part int64,
	        decimal_time_part_ns int32,
	        chaotic_part_seed string,
        ) ([]byte, error)
   ```

### Examples

#### Using the standard mode
```golang
package main

import (
	"fmt"
	. "github.com/flow-labs/u3idgo"
	"log"
)

func main() {
	id, err := U3id_s(32, 10, 128)

	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("%x", id)
}

```

Output:
```bash
635aec917a830da65418e9c0a6b4443a
```

#### Using the chaotic mode where the id has no time component
```golang
package main

import (
	"fmt"
	. "github.com/flow-labs/u3idgo"
	"log"
)

func main() {
	id, err := U3id_c(0, 0, 128, "It’s not a bug — it’s a feature.")

	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("%x", id)
}

```

Output:
```bash
377a3236883e3b75db56655d489e6e30
```

#### Using the time mode with nanosecond precision
```golang
package main

import (
	"fmt"
	. "github.com/flow-labs/u3idgo"
	"log"
)

func main() {
	id, err := U3id_t(
		32,
		30,
		128,
		100,
		999999999,
	)

	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("%x", id)
}

```

Output:
```bash
00000064fffffff859043174ef8e0400
```


#### Using the mode where the user provides everything
```golang
package main

import (
	"fmt"
	. "github.com/flow-labs/u3idgo"
	"log"
)

func main() {
	id, err := U3id_a(
		32,
		30,
		128,
		100,
		999999999,
		"It works on my machine.",
	)

	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("%x", id)
}

```

Output:
```bash
00000064fffffffb36d622abec775d00
```

## Developing

Do not edit the files in the github.com/flow-labs/u3idgo repo. This repo is automatically generated as a copy of the files within the original u3id repo. If you would like to develop this module, edit the files in the github.com/flow-labs/u3id-cython/u3idgo repo and then run the make command to publish to this repo. This is done so that the original C code is the source of truth and there aren't two separately developed copies.