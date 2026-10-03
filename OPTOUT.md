# TDM and robots filtering

`warc2text` discards records based on Text and Data Mining (TDM) reservation
protocol and robots exclusion mechanisms. Both HTTP response headers
and HTML `<meta>` tags are processed to find relevant opt-out signals.
For simplicity and robustness, we discard all records containing any opt-out
signals with only few (frequently occurring) exceptions that are safe to keep. 

## TDM reservation

* HTTP header `tdm-reservation`: if present, the record is discarded unless the
  value is exactly `0` (meaning no reservation).
* Meta tag `<meta name="tdm-reservation" content="...">`: the record is
  discarded if *any* matching tag has a value other than `0`.


## Robots reservation

Both the `X-Robots-Tag` HTTP header and the `<meta name="robots" ...>` meta tag
hold a comma-separated list of directives.

* HTTP header `X-Robots-Tag`: if present, the record is discarded if its
  value contains a discarding directive.
* Meta tag `<meta name="robots" content="...">`: the record is
  discarded if *any* matching tag contains a dicarding directive.

Based on the list of the most frequent directives, we define a discarding directive
as  **any directive starting with `no`**, except these two frequent and harmless directives:
* `nofollow` (do not follow any links on this page),
* `noodp` (do not use the Open Directory Project description for this page in search snippets). 



