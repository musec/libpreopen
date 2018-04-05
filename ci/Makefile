default: test

CONFIG=	config.ini
JOB_FILES=\
	jenkins.yaml \

test: .last-checked
update: .last-updated

.last-checked: ${CONFIG} ${JOB_FILES}
	jenkins-jobs --conf ${CONFIG} test ${JOB_FILES} > /dev/null
	touch .last-checked

.last-updated: ${CONFIG} ${JOB_FILES}
	jenkins-jobs --conf ${CONFIG} update ${JOB_FILES} > /dev/null
	touch .last-updated

