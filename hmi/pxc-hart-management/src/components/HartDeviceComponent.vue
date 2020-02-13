<template>
    <div class="hart-device row">
        <section class="col-md-8">
            <h2>Device Summary</h2>
            <div class="row">
                <div class="col-md-3 col-sm-12">
                    <img src="../assets/img/dummy-device.jpg" class="img-fluid"/>
                </div>
                <div class="col-md-9">
                    <p v-for="(attr, key) in hartAttrs" v-bind:key="key">
                        <b>{{ attr }}:</b> {{ device[key] }}
                    </p>
                </div>
            </div>
            <div class="card" style="width: 18rem;">
                <img class="card-img-top" src="../assets/img/dummy-device.jpg" alt="Card image cap">
                <div class="card-body">
                    <h5 class="card-title">{{ device.longTag }}</h5>
                    <h6 class="card-subtitle mb-2 text-muted">{{ device.name + ' | ' + device.company }}</h6>
                    <p v-for="(attr, key) in hartAttrs" v-bind:key="key">
                        {{ attr }}: {{ device[key] }}
                    </p>
                </div>
            </div>
        </section>
        <aside class="col-md-4 col-sm-12">
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

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, verboseHartDeviceAttrs } from '../types'

@Component({
    components: {
    }
})
export default class HartDeviceComponent extends Vue {
    @Prop() device: HartDeviceDto
    public hartAttrs = verboseHartDeviceAttrs
}
</script>

<style scoped>
  
</style>