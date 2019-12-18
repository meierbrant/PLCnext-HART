<template>
    <div class="hart-device row">
        <section class="col-md-8">
            <h2>Device Summary</h2>
            <div class="row">
                <div class="col-md-3">
                    <img src="../assets/img/dummy-device.jpg" class="img-fluid"/>
                </div>
                <div class="col-md-9">
                    <p>
                        <b>Long tag:</b> {{ longTag }}
                    </p>
                    <p>
                        <b>HART revision:</b> {{ hartRev }}
                    </p>
                </div>
            </div>
        </section>
        <aside class="col-md-4">
            <h2>Send a Command</h2>
            <div id="send-cmd">
                <div class="input-group mb-3">
                    <input type="text" class="form-control" placeholder="command #" aria-label="cmdnum" aria-describedby="basic-addon2">
                    <div class="input-group-append">
                        <button class="btn btn-outline-secondary" type="button">Send</button>
                    </div>
                </div>
                <div id="cmd-result">
                    <h3>Response</h3>
                    <p data-bind="text: cmdResponse.data"></p>
                </div>
            </div>
        </aside>
    </div>
</template>

<script>
module.exports = {
  props: ['longTag', 'hartRev']
}
</script>

<style scoped>
  
</style>