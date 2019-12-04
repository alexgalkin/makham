# # Very simple workflow

# `ingress` shows the data needed to start the workflow. Example data points follow
path = ingress "/path/to/api"
# `GET` can be used to simulate a HTTP request. Return status and example data values
# follow
request = GET http://api.example.com${path}/ 200 {"example": "data"}
# `await` is used to wait for asynchronous data (like from `GET` and other HTTP requests
await request:
    # `schema-check` will check values against a schema
    schema-check ${request} {"properties": {"example": {"const": "data"}}}
    # The exit data from the workflow
    exit request