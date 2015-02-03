import sys

def wsgi_test(environ, start_response):
    status = "200 OK"
    headers = [('Content-Type', 'text/html')]
    start_response(status, headers)

    errors = environ['wsgi.errors']
    errors.flush()
    errors.write('test errors\n')
    errors.flush()
    errors.writelines(['test 1\n', 'test 2\n', 'test 3\n'])

    yield '<u>Input:</u><br />'
    input = environ['wsgi.input']
    line_nr = 0
    for line in input:
        yield '[%i] %s<br />' % (line_nr, line)
        line_nr += 1

    yield '<br />Total:<br />' + input.read() + '<br />'

    yield '<br /><u>Environment:</u><br />'
    for key, value in environ.iteritems():
        yield '%s: %s<br />' % (key, value)