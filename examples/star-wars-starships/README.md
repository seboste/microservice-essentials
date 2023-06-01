# Star-Wars-Starships

This simple microservice aims to illustrate the usage of many features of the **Microservice essentials** library such as incoming and outgoing requests, graceful shutdown, logging, configuration via environment variables, and basic authentication and authorization.

## Functionality

The API of the microservice provides endpoints for listing star ships, getting information about a star ship and setting the status of a star ship. It is based on the [Star Wars API](https://swapi.dev).

The microservice can be called via a HTTP (REST)-handler or, alternatively, via a gRPC handler. State is stored in a simple key-value based in-memory-database. An HTTP-library is used for making calls to the [Star Wars API](https://swapi.dev).

## Architecture

The microservice follows the hexagonal (aka ports & adapters) architecture pattern. The business logic is located inside the *core* folder and does only depend on abstract interfaces and business entities defined in the *ports* folder. *Adapters* implement those interfaces based on specific technologies. The *app* contains the main function that configures and instantiates the *adapters* and injects them into the *core*. 

The resulting boilerplate code is in many cases outweighted by the inherently flexible nature of this archtectural style as fast-living technologies can easily be exchanged without impacting the business logic.