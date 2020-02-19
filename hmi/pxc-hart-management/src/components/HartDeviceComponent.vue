<template>
    <div class="hart-device">
        <div class="row">
            <div class="col-md-6"> <!-- left column -->
                <div class="row">
                    <section class="">
                        <h2>Device Summary</h2>
                        <div class="row">
                            <div class="col-md-4 col-sm-12">
                                <img src="../assets/img/dummy-device.jpg" class="img-fluid"/>
                            </div>
                            <div class="col-md-8">
                                <table>
                                    <tr v-for="(attr, key) in hartAttrs" v-bind:key="key">
                                        <th style="text-align: right; padding-right: 0.5em;">{{ attr }}</th>
                                        <td style="text-align: left; padding-right: 0.5em;">{{ device[key] }}</td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                    </section>
                </div>
                <br>
                <div class="row">
                    <section class="">
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
                    </section>
                </div>
            </div>
            <div class="col-md-6"> <!-- right column-->
                <section class="var-grid">
                    <h2>Vars</h2>
                    <div class="row">
                        <div class="col-sm-6" v-for="(v, key) in vars" v-bind:key="key">
                            <b-card :title="key" class="var-card">
                                <div class="value text-secondary">
                                    {{ v.value.toFixed(2) }}<span class="units text-dark">{{ v.units }}</span>
                                </div>
                            </b-card>
                        </div>
                    </div>
                </section>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, verboseHartDeviceAttrs, HartVars } from '../types'

@Component({
    components: {
    }
})
export default class HartDeviceComponent extends Vue {
    @Prop() device: HartDeviceDto
    public hartAttrs = verboseHartDeviceAttrs

    get vars(): HartVars {
        return this.device.vars
    }
}
</script>

<style scoped>
  
</style>