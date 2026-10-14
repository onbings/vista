# Context

To create the XTS "emotion OS" package, we will need to bundle multiple artifacts into a single package.

Some of those artifacts are actually applications that will need to interact with each others.

For each of those applications we want 
  - To build and version them independently
  - To avoid rebuilding an application that does not need to

Because each applications are independent, how do we make sure that they are "compatible" ? 

Suppose we have 2 applications A and B, that are both using an RPC library X to communicate : how do we ensure that selected versions for both A and B incorporate the same version of X (or at least a "compatible" version) ?

Failing to detect an "incompatible" situation means we will create a package that will either directly crash at runtime or worse behave incorrectly.

In order to assess this compatibility at packaging time we will need some information representing the various **contracts** an application have.

The problem is thus : how do we represent those **contracts** ?

# CMake custom transitive properties

Because those **contracts** can come from libraries we need a way to propagate them accross the build system.

Since CMake 3.30, it's possible to define and propagate custom LINK or COMPILE requirements through [custom transitive properties](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#custom-transitive-properties).

In our case, our **contracts** are precisely LINK time requirements by nature.

## Expressing custom requirement 

Suppose we have 2 libraries A and B, each defining a contract and an application E that links with both A and B.

```cpp
add_library(A ...)

set_target_properties(A
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"      # Tell CMake the name of dependencies to propagate when doing 'target_link_libraries' with our target
    INTERFACE_EVS_CONTRACTS    "contract_a=1.1.1.1" # Need to populate INTERFACE_* which is the one propagated to dependees
)

add_library(B ...)

set_target_properties(B
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"      # Tell CMake the name of dependencies to propagate when doing 'target_link_libraries' with our target 
    INTERFACE_EVS_CONTRACTS    "contract_b=2.2.2.2" # Need to populate INTERFACE_* which is the one propagated to dependees
)

add_executable(E ...)

set_target_properties(B
  PROPERTIES
    EVS_CONTRACTS "contract_e=1.2.3.4"              # No need for "INTERFACE_" as an executable is the final destination
)

target_link_libraries(E
  PRIVATE
    A
    B
)

add_custom_target(print ALL VERBATIM
  COMMAND ${CMAKE_COMMAND} -E echo "Contracts = $<TARGET_PROPERTY:E,EVS_CONTRACTS>"
)

# print command should show output like :
#  Contracts = contract_e=1.2.3.4;contract_a=1.1.1.1;contract_b=2.2.2.2
```

Meaning we can retrieve at build time of E the aggregated **contracts** it inherited from A and B by linking with them !

# Proposal

We propose

  - To name the custom CMake property **EVS_CONTRACTS** to forward contracts in the build system
  - To define a contract field as a JSON value **{ "name":`<name>`, "version":`<version>`, ["policy":`<policy>`] }** to potentially extend it later if needed where
      - **name** is the name of the contract
      - **version** is the version of the contract in the form **major.minor.patch.build**
      - **policy** is an optional field indicating compatibility requirement with consumers with the following possible values
          - **exact** : The version must match exactly with every other contract consumers
          - **same_major** : The major version must match with every other contract consumers
          - **same_minor** : The major and minor version must match with every other contract consumers
          - **any_greater** : The consumer side must have a greater version number than the contract defined
  - Each contract name can be freely defined but should be unique in the whole emotion-os package

It's thus the responsability of each project maintainer to define contracts as needed and set the **EVS_CONTRACTS** property on the CMake targets it produce.

## Examples

### An application (server) and a library (client)

I build an application evs-hwfw-r6X and a client library evs-hwfw-r6x-client

  - I define a contract named **evs-hwfw-r6x** (arbitrary but name must be unique)
  - I choose policy to be **same_minor** on the server side
  - I set properties on targets

```cpp
# Application
set_target_properties(evs-hwfw-r6X
  PROPERTIES
    EVS_CONTRACTS "{ \"name\":\"evs-hwfw-r6x\", \"version\":\"${PROJECT_VERSION}\", \"policy\": \"same_minor\" }"
)
```

```cpp
# Library
set_target_properties(evs-hwfw-r6x-client
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"
    INTERFACE_EVS_CONTRACTS    "{ \"name\":\"evs-hwfw-r6x\", \"version\":\"${PROJECT_VERSION}\" }"
)
```

### A library (server) and a library (client)

I build a server library evs-xts-south-api-ingests and client library named evs-xts-south-api-ingests-client

  - I define a contract named **evs-xts-south-api-ingests** (arbitrary but name must be unique)
  - I choose policy to be **exact** on the server side
  - I set properties on targets

```cpp
# Server library
set_target_properties(evs-xts-south-api-ingests
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"
    INTERFACE_EVS_CONTRACTS    "{ \"name\":\"evs-xts-south-api-ingests\", \"version\":\"${PROJECT_VERSION}\", \"policy\": \"exact\" }"
)
```
```cpp
# Client library
set_target_properties(evs-xts-south-api-ingests-client
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"
    INTERFACE_EVS_CONTRACTS    "{ \"name\":\"evs-xts-south-api-ingests\", \"version\":\"${PROJECT_VERSION}\" }"
)
```

### A single library implementing both client and server

I build a library named evs-hwfw-npp that includes both an RPC client and server

  - I define a contract name **evs-hwfw-npp** (arbitrary but must be unique)
  - I choose policy to be **exact**
  - I set properties on targets
  
```cpp
set_target_properties(evs-hwfw-npp
  PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EVS_CONTRACTS"
    INTERFACE_EVS_CONTRACTS    "{ \"name\":\"evs-hwfw-npp\", \"version\":\"${PROJECT_VERSION}\", \"policy\": \"exact\" }"
)
```

#### Important remarks

Exercice caution when defining contracts on low-level components such as above because

  1. Not all policies are revelant as there is no way to disambiguate consumer from producer (e.g. 'any_greater' will be equivalent to 'exact' as (A >= B && B >= A) is only true if A == B).
  2. It will require all applications using the component to be compatible even if they do not talk to each other ! This might not necessarily be want you want.

In the current example, it would probably be better to not define any contract on evs-hwfw-npp but rather define it at a higher level in the component using it to implement a concrete protocol.

Still it might be useful to enforce that a component should be the same in all application using it if needed.

## Collecting contracts

When building an application, we are at the end of the chain.

We can thus retrieve the property **EVS_CONTRACTS** and generate a manifest file through a add_custom_command

See [manifest_proposal](manifest_proposal) for a definition of manifest file format.

```cpp
set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/manifest.json)

add_custom_command(
    OUTPUT  ${OUTPUT_FILE}
    COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Manifest/generate_manifest.py 
            --contracts $<TARGET_PROPERTY:${TARGET_NAME},EVS_CONTRACTS>      # Collect the list of contracts for the application
            ...
    DEPENDS ${TARGET_NAME}
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Manifest/generate_manifest.py
    COMMENT "Generating manifest file for target ${TARGET_NAME} at ${OUTPUT_FILE}"
    VERBATIM
  )
```

## Enforcing Contracts

When packaging applications we can use the following algorithm

- For each application deployed, retrieve its manifest if any
- For each manifest, parse the **contracts** and match the ones with a defined policy against the ones in other manifest with the same name.

### Example

```cpp
# app-A-manifest.json
{
  "name" : "A",
  "version" : "1.2.3.4",
  "contracts" : [
    {
      "name" : "contract_1",
      "version": "4.5.6.7",
      "policy": "exact"
    },
    ...
  ]
}

# app-B-manifest.json
{
  "name" : "B",
  "version" : "0.0.1.42",
  "contracts" : [
    {
      "name" : "contract_1",
      "version": "4.7.8.9"
    },
    ...
  ]
}
```

Could produce an error like

```text
[Failure] >> Application "A" requires contract "contract_1" to match exactly version "4.5.6.7" but application "B" uses "contract_1" at version "4.7.8.9". Applications are not compatible !
```
